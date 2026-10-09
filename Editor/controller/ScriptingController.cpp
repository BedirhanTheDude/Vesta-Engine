#include "ScriptingController.h"

#include <controller/SceneController.h>

#include <core/Project.h>
#include <core/Application.h>
#include <scene/components/ComponentFactory.h>
#include <scripting/ScriptCompiler.h>
#include <scripting/ScriptLoader.h>

#include <mutex>
#include <thread>

namespace ScriptingController {

	static void compileTask() {
		bool compiled = ScriptCompiler::compile();
		if (compiled) {
			std::mutex& sceneMutex = Project::getSceneMutex();

			// lock the Scene so it doesn't try to access unloaded Scripts
			std::lock_guard<std::mutex> lock(sceneMutex);

			ScriptCompiler::reloadScripts(*Application::getCurrentScene());
		}
	}

	// NOTE: Script compiler is currently blocking
	void compileScripts() {
		std::thread compileThread(compileTask);

		compileThread.detach();
	}

	bool createNewScript(const std::string& scriptName, const std::string& path) {
		return ScriptLoader::createNewScriptFile(scriptName, path);
	}
}