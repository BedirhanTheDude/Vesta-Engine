#include "ScriptLoader.h"

#include <sstream>
#include <fstream>

std::unordered_map<std::string, std::filesystem::path> ScriptLoader::sourcePaths;
std::unordered_map<std::string, std::filesystem::path> ScriptLoader::headerPaths;

void ScriptLoader::loadScriptPaths() {
	if (!sourcePaths.empty() || !headerPaths.empty()) return;

	auto currentPath = std::filesystem::current_path();
	auto scriptsPath = currentPath / "scripts";

	if (!std::filesystem::exists(scriptsPath)) return;

	for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(scriptsPath)) {
		if (!dirEntry.is_regular_file() || dirEntry.path().filename() == "ScriptsDLL.cpp") continue;

		std::ifstream file(dirEntry.path());
		if (!file.is_open()) continue;

		std::string buf;

		if (dirEntry.path().extension().string() == ".h") {
			bool scriptBegin = false;
			bool scriptEnd = false;
			while (std::getline(file, buf)) {
				size_t pos1 = buf.find("END_SCRIPT"); // check END_SCRIPT first because SCRIPT is a subset
				if (pos1 != std::string::npos) {
					scriptEnd = true;
					if (scriptBegin && scriptEnd) break;

					continue;
				}

				size_t pos2 = buf.find("SCRIPT");
				if (pos2 != std::string::npos) {
					scriptBegin = true;
					if (scriptBegin && scriptEnd) break;
				}
			}

			if (scriptBegin && scriptEnd) {
				std::string name = dirEntry.path().stem().string();
				headerPaths[name] = std::filesystem::relative(dirEntry.path(), currentPath);

				auto sourcePath = dirEntry.path().parent_path() / (name + ".cpp");
				if (std::filesystem::exists(sourcePath))
					sourcePaths[name] = std::filesystem::relative(sourcePath, currentPath);
			}
		}
	}
}

std::string ScriptLoader::buildCompileCommandString() {
	std::ostringstream cmd;

	cmd << "compiler\\clang-cl.exe /LD ";
	cmd	<< "scripts\\ScriptsDLL.cpp ";

	ScriptLoader::loadScriptPaths();

	for (auto& [name, path] : sourcePaths)
		cmd << path.string() << " ";

	cmd << "/I include ";

#ifdef _DEBUG
	cmd << "/MDd ";
#else
	cmd << "/MD ";
#endif

	cmd << "/Zi ";

	cmd << "/std:c++17 /EHsc ";

	cmd << "/link Engine.lib /DEBUG /PDB:scripts.pdb /out:temp_scripts.dll";

	return cmd.str();
}

bool ScriptLoader::scriptsDLLExists() {
	return std::filesystem::exists(std::filesystem::current_path() / "scripts.dll");
}

void ScriptLoader::replaceOldDLLFile() {
	auto DLLPath = std::filesystem::current_path() / "scripts.dll";
	std::filesystem::remove(DLLPath);
	std::filesystem::rename(std::filesystem::current_path() / "temp_scripts.dll", DLLPath);
}

void ScriptLoader::buildDLLSourceFile() {
	std::filesystem::path dllSourcePath = std::filesystem::current_path() / "scripts" / "ScriptsDLL.cpp";
	
	ScriptLoader::loadScriptPaths();
	std::ofstream file(dllSourcePath);

	if (!file.is_open()) return;

	file << "#include <Windows.h>\n";
	
	for (auto& [name, path] : headerPaths) {
		auto headerPath = path.parent_path() / (path.stem().string() + ".h");
		auto includePath = std::filesystem::relative(headerPath, dllSourcePath.parent_path());
		file << "#include " << "\"" << includePath.generic_string() << "\"\n";
	}

	file << "extern \"C\" __declspec(dllexport) void registerComponents() {}\n";
	file << "BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) { return TRUE; }\n";

	file.close();
}