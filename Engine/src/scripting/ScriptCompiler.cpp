#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include <Windows.h>

#include "ScriptCompiler.h"
#include "ScriptLoader.h"
#include "ScriptCache.h"

#include <scene/components/ComponentFactory.h>
#include <scene/Scene.h>

#include <scene/system/ScriptSystem.h>

#include <core/Project.h>

#include <iostream>
#include <system_error>

HMODULE ScriptCompiler::currentDLLHandle;
bool ScriptCompiler::pollingEnabled = true;

// the bundled compiler, include/ and Engine.lib are next to the executable
static std::wstring getExecutableDirectory() {
	std::wstring buffer(MAX_PATH, L'\0');

	for (;;) {
		DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
		if (length == 0) return {};

		if (length < buffer.size()) {
			buffer.resize(length);
			break;
		}

		buffer.resize(buffer.size() * 2); // truncated
	}

	return std::filesystem::path(buffer).parent_path().wstring();
}

bool ScriptCompiler::compile() {
	ScriptLoader::clearScriptCache();
	if (!ScriptLoader::buildDLLSourceFile()) return false;

	std::wstring cmd;
	if (!ScriptLoader::buildCompileCommandString(cmd)) return false;

	std::wstring workingDir = getExecutableDirectory();
	if (workingDir.empty()) return false;

	STARTUPINFOW si = {};
	PROCESS_INFORMATION pi = {};

	si.cb = sizeof(si);

	// CreateProcessW may write into the command line buffer, cmd.data() is writable in C++17
	if (!CreateProcessW(nullptr, cmd.data(), nullptr, nullptr,
		FALSE, 0, nullptr, workingDir.c_str(), &si, &pi)) {
		std::cerr << "[Error] Could not start the script compiler (error " << GetLastError() << ")\n";
		return false;
	}

	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD exitCode = 0;
	GetExitCodeProcess(pi.hProcess, &exitCode);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return exitCode == 0;
}

HMODULE ScriptCompiler::loadDLL() {
	if (!ScriptLoader::scriptsDLLExists()) return currentDLLHandle = nullptr;

	currentDLLHandle = LoadLibraryW(ScriptLoader::getDLLPath().wstring().c_str());

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

// NOTE: Cannot run concurrently with Scene update
HMODULE ScriptCompiler::reloadScripts(Scene& scene) {
	ScriptCache::saveScriptState(scene);
	ScriptSystem::clearScripts(scene);
	ComponentFactory::clearScriptRegistry();

	ScriptCompiler::unloadDLL(ScriptCompiler::getCurrentDLLHandle());
	ScriptLoader::replaceOldDLLFile(); // on failure the old DLL is still there and gets loaded again

	ScriptCompiler::loadDLL(); // static lambdas reinitialize the script registry
	ScriptCache::loadScriptState(scene);

	return currentDLLHandle;
}

void ScriptCompiler::poll(float dt, Scene& scene) {
	if (!pollingEnabled) return;

	if (!Project::isOpen()) return;

	static float timer = 0.0f;
	static bool hasBaseline = false;
	static std::filesystem::file_time_type lastWriteTime = {};

	timer += dt;
	if (timer < 1.0f) return;
	timer = 0.0f;

	// TODO: replace this whole-project scan with a directory watcher
	std::filesystem::file_time_type latestWrite = {};

	std::error_code ec;
	std::filesystem::recursive_directory_iterator it(Project::getRoot(),
		std::filesystem::directory_options::skip_permission_denied, ec);
	const std::filesystem::recursive_directory_iterator end;

	// generated files live in the ignored cache directory, so the build can't trigger itself
	for (; !ec && it != end; it.increment(ec)) {
		const std::filesystem::directory_entry& entry = *it;
		std::error_code entryEc;

		if (entry.is_directory(entryEc)) {
			if (Project::isIgnoredDirectory(entry.path()))
				it.disable_recursion_pending();
			continue;
		}

		if (!ScriptLoader::isScriptSourceFile(entry.path())) continue;

		auto t = entry.last_write_time(entryEc);
		if (!entryEc && t > latestWrite) latestWrite = t;
	}

	if (ec) return;

	// the first scan only records what's there
	if (!hasBaseline) {
		hasBaseline = true;
		lastWriteTime = latestWrite;
		return;
	}

	if (latestWrite > lastWriteTime) {
		lastWriteTime = latestWrite;
		reloadScripts(scene);
	}
}