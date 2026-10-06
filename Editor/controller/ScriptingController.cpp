#include "ScriptingController.h"

#include <controller/SceneController.h>

#include <core/Project.h>
#include <scripting/ScriptCompiler.h>
#include <scripting/ScriptLoader.h>

#include <mutex>
#include <thread>

namespace ScriptingController {

	static void compileTask(bool* outCompiled) {
		std::mutex& sceneMutex = Project::getSceneMutex();

		// lock the Scene so it doesn't try to access unloaded Scripts
		std::lock_guard<std::mutex> lock(sceneMutex);

		bool compiled = ScriptCompiler::compile();
		if (compiled) {
			ScriptCompiler::unloadDLL(ScriptCompiler::getCurrentDLLHandle());
			ScriptLoader::replaceOldDLLFile();
			ScriptCompiler::loadDLL();
		}

		if (outCompiled) *outCompiled = compiled;
	}

	// NOTE: Script compiler is currently blocking
	void compileScripts(bool* outCompiled) {
		std::thread compileThread(compileTask, outCompiled);

		compileThread.detach();
	}

	bool createNewScript(const std::string& scriptName, const std::string& path) {
		return ScriptLoader::createNewScriptFile(scriptName, path);
	}
}