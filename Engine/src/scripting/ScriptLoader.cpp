#include "ScriptLoader.h"

#include <core/Project.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

std::unordered_map<std::string, fs::path> ScriptLoader::sourcePaths;
std::unordered_map<std::string, fs::path> ScriptLoader::headerPaths;

// a header is a script if it has both SCRIPT and END_SCRIPT in it
static bool isScriptHeader(const fs::path& path) {
	std::ifstream file(path);
	if (!file.is_open()) return false;

	bool scriptBegin = false;
	bool scriptEnd = false;

	std::string buf;
	while (std::getline(file, buf)) {
		if (buf.find("END_SCRIPT") != std::string::npos) // check END_SCRIPT first because SCRIPT is a subset
			scriptEnd = true;
		else if (buf.find("SCRIPT") != std::string::npos)
			scriptBegin = true;

		if (scriptBegin && scriptEnd) return true;
	}

	return false;
}

bool ScriptLoader::isScriptSourceFile(const fs::path& path) {
	const fs::path extension = path.extension();
	return extension == ".h" || extension == ".hpp" || extension == ".cpp";
}

fs::path ScriptLoader::getBuildDirectory() {
	return Project::getCacheDirectory() / "build";
}

fs::path ScriptLoader::getDLLPath() {
	return getBuildDirectory() / "scripts.dll";
}

bool ScriptLoader::loadScriptPaths() {
	if (!headerPaths.empty()) return true;

	if (!Project::isOpen()) {
		std::cerr << "[Error] Scripts can't be loaded without an open project\n";
		return false;
	}

	bool succeeded = true;

	std::error_code ec;
	fs::recursive_directory_iterator it(Project::getRoot(), fs::directory_options::skip_permission_denied, ec);
	const fs::recursive_directory_iterator end;

	for (; !ec && it != end; it.increment(ec)) {
		const fs::directory_entry& entry = *it;
		std::error_code entryEc;

		if (entry.is_directory(entryEc)) {
			if (Project::isIgnoredDirectory(entry.path()))
				it.disable_recursion_pending();
			continue;
		}

		if (!entry.is_regular_file(entryEc) || entry.path().extension() != ".h") continue;
		if (!isScriptHeader(entry.path())) continue;

		// the script's class name is the header's name, it has to be unique across the whole project
		std::string name = entry.path().stem().string();

		auto [existing, inserted] = headerPaths.emplace(name, entry.path());
		if (!inserted) {
			std::cerr << "[Error] Two scripts are named \"" << name << "\": "
				<< existing->second << " and " << entry.path() << '\n';
			succeeded = false;
			continue;
		}

		fs::path sourcePath = entry.path();
		sourcePath.replace_extension(".cpp");
		if (fs::exists(sourcePath, entryEc))
			sourcePaths[name] = std::move(sourcePath);
	}

	if (ec) {
		std::cerr << "[Error] Scanning the project for scripts failed: " << ec.message() << '\n';
		succeeded = false;
	}

	// a partial list must not be mistaken for a complete one by the next call
	if (!succeeded) clearScriptCache();

	return succeeded;
}

bool ScriptLoader::buildCompileCommandString(std::wstring& outCommand) {
	if (!loadScriptPaths()) return false;

	const fs::path buildDir = getBuildDirectory();

	// sources go into a response file, absolute paths of many scripts can hit the command line length limit
	const fs::path responseFilePath = buildDir / "sources.rsp";
	{
		std::ofstream responseFile(responseFilePath);
		if (!responseFile.is_open()) {
			std::cerr << "[Error] Could not write " << responseFilePath << '\n';
			return false;
		}

		responseFile << '"' << (buildDir / "ScriptsDLL.cpp").generic_string() << "\"\n";
		for (auto& [name, path] : sourcePaths)
			responseFile << '"' << path.generic_string() << "\"\n";
	}

	std::wostringstream cmd;

	cmd << L"compiler\\clang-cl.exe /nologo /LD ";
	cmd << L"@\"" << responseFilePath.generic_wstring() << L"\" ";

	cmd << L"/I include ";
	cmd << L"/I \"" << Project::getRoot().generic_wstring() << L"\" ";

#ifdef _DEBUG
	cmd << L"/MDd ";
#else
	cmd << L"/MD ";
#endif

	cmd << L"/Zi ";

	cmd << L"/std:c++17 /EHsc ";

	cmd << L"/Fo\"" << buildDir.generic_wstring() << L"/\" ";

	cmd << L"/link Engine.lib /DEBUG ";
	cmd << L"/PDB:\"" << (buildDir / "scripts.pdb").generic_wstring() << L"\" ";
	cmd << L"/out:\"" << (buildDir / "temp_scripts.dll").generic_wstring() << L"\"";

	outCommand = cmd.str();
	return true;
}

bool ScriptLoader::scriptsDLLExists() {
	if (!Project::isOpen()) return false;

	std::error_code ec;
	return fs::exists(getDLLPath(), ec);
}

bool ScriptLoader::replaceOldDLLFile() {
	const fs::path dllPath = getDLLPath();

	// fails while the old DLL is still loaded
	std::error_code ec;
	fs::remove(dllPath, ec);
	if (ec) {
		std::cerr << "[Error] Could not remove " << dllPath << ": " << ec.message() << '\n';
		return false;
	}

	fs::rename(getBuildDirectory() / "temp_scripts.dll", dllPath, ec);
	if (ec) {
		std::cerr << "[Error] Could not replace " << dllPath << ": " << ec.message() << '\n';
		return false;
	}

	return true;
}

bool ScriptLoader::buildDLLSourceFile() {
	if (!loadScriptPaths()) return false;

	const fs::path buildDir = getBuildDirectory();

	std::error_code ec;
	fs::create_directories(buildDir, ec);
	if (ec) {
		std::cerr << "[Error] Could not create " << buildDir << ": " << ec.message() << '\n';
		return false;
	}

	const fs::path dllSourcePath = buildDir / "ScriptsDLL.cpp";
	std::ofstream file(dllSourcePath);

	if (!file.is_open()) {
		std::cerr << "[Error] Could not write " << dllSourcePath << '\n';
		return false;
	}

	file << "#include <Windows.h>\n";

	for (auto& [name, path] : headerPaths)
		file << "#include \"" << path.generic_string() << "\"\n";

	file << "extern \"C\" __declspec(dllexport) void registerComponents() {}\n";
	file << "BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) { return TRUE; }\n";

	return true;
}
