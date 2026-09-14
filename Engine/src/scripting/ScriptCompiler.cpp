#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#include "ScriptCompiler.h"
#include "ScriptLoader.h"

#include <scene/components/ComponentFactory.h>
#include <persistance/SceneSerializer.h> // !!!
#include <scene/Scene.h> // remove this tight coupling as soon as possible

HMODULE ScriptCompiler::currentDLLHandle;
bool ScriptCompiler::pollingEnabled = true;

bool ScriptCompiler::compile() {
	ScriptLoader::clearScriptCache();
	ScriptLoader::buildDLLSourceFile();

	STARTUPINFOA si = {};
	PROCESS_INFORMATION pi = {};

	si.cb = sizeof(si);
	std::string cmd = ScriptLoader::buildCompileCommandString();

	std::string workingDir = ScriptLoader::getWorkingDirectoryString();
	if (!CreateProcessA(nullptr, cmd.data(), nullptr, nullptr,
		FALSE, 0, nullptr, workingDir.c_str(), &si, &pi)) return false;

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exitCode == 0;
}

// assumes the dll is always next to the binary
HMODULE ScriptCompiler::loadDLL() {
	std::filesystem::path dllPath = std::filesystem::current_path() / "scripts.dll";
	currentDLLHandle = LoadLibraryA(dllPath.string().c_str());

	if (currentDLLHandle) {
		using RegisterFn = void(*)();
		RegisterFn registerComponents = (RegisterFn)GetProcAddress(currentDLLHandle, "registerComponents");
		if (registerComponents) registerComponents();
	}

	return currentDLLHandle;
}

void ScriptCompiler::unloadDLL(HMODULE handle) {
	FreeLibrary(handle);
	currentDLLHandle = {};
}

HMODULE ScriptCompiler::getCurrentDLLHandle() { return currentDLLHandle; }

HMODULE ScriptCompiler::reloadScripts(Scene& scene) {
	if (!ScriptCompiler::compile()) return currentDLLHandle;

	SceneSerializer::save(scene, true); // save to temp
	scene.clear();

	ComponentFactory::clearScriptRegistry();

	ScriptCompiler::unloadDLL(ScriptCompiler::getCurrentDLLHandle());
	ScriptLoader::replaceOldDLLFile();

	ScriptCompiler::loadDLL(); // static lambdas reinitialize the script registry

	SceneSerializer::load(scene, true);

	return currentDLLHandle;
}

void ScriptCompiler::poll(float dt, Scene& scene) {
	if (!pollingEnabled) return;

	static float timer = 0.0f;
	static std::filesystem::file_time_type lastWriteTime = {};

	timer += dt;
	if (timer < 1.0f) return;
	timer = 0.0f;

	auto scriptsPath = std::filesystem::current_path() / "scripts";
	if (!std::filesystem::exists(scriptsPath)) return;

	std::filesystem::file_time_type latestWrite = {};
	for (const auto& entry : std::filesystem::recursive_directory_iterator(scriptsPath)) {
		if (!entry.is_regular_file()) continue;
		auto t = entry.last_write_time();
		if (t > latestWrite) latestWrite = t;
	}

	if (latestWrite > lastWriteTime) {
		lastWriteTime = latestWrite;
		if (lastWriteTime != std::filesystem::file_time_type{})
			reloadScripts(scene);
	}
}