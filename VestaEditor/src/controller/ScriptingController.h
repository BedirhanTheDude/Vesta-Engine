#pragma once

#include <string>

namespace ScriptingController {
	void compileScripts();
	bool createNewScript(const std::string& scriptName, const std::string& path);
}