#pragma once

#include <string>

namespace ScriptingController {
	void compileScripts(bool* outCompiled = nullptr);
	bool createNewScript(const std::string& scriptName, const std::string& path);
}