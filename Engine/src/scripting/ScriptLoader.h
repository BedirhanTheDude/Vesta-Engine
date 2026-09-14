#pragma once

#include <string>
#include <filesystem>
#include <unordered_map>

// handles filesystem related tasks for dynamic Script linking
// this is a separate class from ScriptCompiler because Windows.h interferes with std::filesystem

class ScriptLoader {
public:
	static std::string getWorkingDirectoryString() { return std::filesystem::current_path().string(); }
	static std::string buildCompileCommandString();

	static void clearScriptCache() { sourcePaths.clear(); }
	static void buildDLLSourceFile();
	static void replaceOldDLLFile();
	static bool scriptsDLLExists();

private:
	static void loadScriptPaths();

	static std::unordered_map<std::string, std::filesystem::path> sourcePaths;
	static std::unordered_map<std::string, std::filesystem::path> headerPaths;
};