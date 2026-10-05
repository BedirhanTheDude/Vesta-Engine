#include <core/Project.h>

#include <persistance/Archive.h>

#include <scripting/ScriptCompiler.h>
#include <scripting/ScriptLoader.h>

#include <iostream>
#include <system_error>

namespace fs = std::filesystem;

fs::path Project::root;
fs::path Project::cacheDirectory;
std::string Project::name;
std::string Project::startupScene;
bool Project::projectOpen = false;

bool Project::isOpen() { return projectOpen; }
const fs::path& Project::getRoot() { return root; }
const fs::path& Project::getCacheDirectory() { return cacheDirectory; }
const std::string& Project::getName() { return name; }
const std::string& Project::getStartupScene() { return startupScene; }

bool Project::create(const fs::path& rootPath, const std::string& projectName) {
	std::error_code ec;
	fs::create_directories(rootPath, ec);
	if (ec) {
		std::cerr << "[Error] Could not create project directory " << rootPath << ": " << ec.message() << '\n';
		return false;
	}

	if (!fs::exists(rootPath / projectFileName, ec)) {
		Archive arch;
		arch.set("name", projectName);
		arch.set("startupScene", std::string());

		if (!arch.saveToFile(rootPath / projectFileName)) {
			std::cerr << "[Error] Could not write " << projectFileName << " in " << rootPath << '\n';
			return false;
		}
	}

	return open(rootPath);
}

bool Project::open(const fs::path& rootPath) {
	std::error_code ec;
	fs::path absolute = fs::weakly_canonical(fs::absolute(rootPath, ec), ec);

	// TODO: Log errors after Console integration
	if (ec || !fs::is_directory(absolute, ec)) {
		std::cerr << "[Error] Project path is not a directory: " << rootPath << '\n';
		return false;
	}

	Archive arch;
	if (!arch.loadFromFile(absolute / projectFileName)) {
		std::cerr << "[Error] No readable " << projectFileName << " in " << absolute << '\n';
		return false;
	}

	std::string projectName = absolute.filename().string();
	std::string scene;
	arch.get("name", projectName);
	arch.get("startupScene", scene);

	fs::path cache = absolute / cacheDirectoryName;
	fs::create_directories(cache, ec);
	if (ec) {
		std::cerr << "[Error] Could not create " << cache << ": " << ec.message() << '\n';
		return false;
	}

	root = std::move(absolute);
	cacheDirectory = std::move(cache);
	name = std::move(projectName);
	startupScene = std::move(scene);
	projectOpen = true;

	bool compiledScripts = ScriptCompiler::compile();
	if (compiledScripts) {
		ScriptLoader::replaceOldDLLFile();
		ScriptCompiler::loadDLL();
	}

	return true;
}

bool Project::resolve(const std::string& relativePath, fs::path& outPath) {
	if (!projectOpen || relativePath.empty()) return false;

	fs::path relative = fs::u8path(relativePath).lexically_normal();

	// a scene file is not allowed to reach outside of the project
	if (relative.has_root_name() || relative.has_root_directory()) return false;
	if (relative.empty() || *relative.begin() == "..") return false;

	outPath = root / relative;
	return true;
}

bool Project::toRelative(const fs::path& path, std::string& outRelativePath) {
	if (!projectOpen) return false;

	std::error_code ec;
	fs::path absolute = fs::absolute(path, ec);
	if (ec) return false;

	fs::path relative = absolute.lexically_normal().lexically_relative(root);
	if (relative.empty() || relative == "." || *relative.begin() == "..") return false;

	outRelativePath = relative.generic_string();
	return true;
}

bool Project::isIgnoredDirectory(const fs::path& directory) {
	const fs::path fileName = directory.filename();
	if (fileName.empty()) return false;

	const fs::path::string_type& str = fileName.native();
	return !str.empty() && str[0] == '.';
}

void Project::setStartupScene(const std::string& relativeScenePath) {
	if (!projectOpen || startupScene == relativeScenePath) return;

	startupScene = relativeScenePath;
	save();
}

bool Project::save() {
	Archive arch;
	arch.set("name", name);
	arch.set("startupScene", startupScene);

	if (!arch.saveToFile(root / projectFileName)) {
		std::cerr << "[Error] Could not write " << (root / projectFileName) << '\n';
		return false;
	}

	return true;
}
