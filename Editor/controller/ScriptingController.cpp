#include "ScriptingController.h"

#include <controller/SceneController.h>

#include <scripting/ScriptCompiler.h>
#include <scripting/ScriptLoader.h>

namespace ScriptingController {

	// NOTE: Script compiler is currently blocking
	bool compileScripts() {
		bool compiled = ScriptCompiler::compile();
		if (compiled) {
			SceneController::stopScene();
			ScriptCompiler::unloadDLL(ScriptCompiler::getCurrentDLLHandle());
			ScriptLoader::replaceOldDLLFile();
			ScriptCompiler::loadDLL();
		}

		return compiled;
	}
}