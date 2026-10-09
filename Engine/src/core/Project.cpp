#include <core/Project.h>

#include <core/Application.h>
#include <persistance/Archive.h>

#include <scene/Scene.h>

#include <scripting/ScriptCompiler.h>
#include <scripting/ScriptLoader.h>

#include <system_error>
#include <filesystem>
#include <iostream>
#include <cstdint>
#include <utility>
#include <string>
#include <mutex>

namespace fs = std::filesystem;

fs::path Project::root;
fs::path Project::cacheDirectory;
fs::path Project::projectArchivePath;
std::string Project::name;
std::string Project::startupScene;
std::mutex Project::sceneMutex;
bool Project::projectOpen = false;

// These are here because Project doesn't need to know about Archive
static Archive copyComponentCache;
static Archive copyEntityCache;
static bool copiedTransform = false;
static bool copiedComponent = false;
static bool copiedEntity = false;

static bool tryGetProjectArchivePath(const std::filesystem::path rootPath, std::filesystem::path& outPath) {
	auto options = std::filesystem::directory_options::skip_permission_denied;
	std::error_code ec;

	auto it = std::filesystem::recursive_directory_iterator(rootPath, options, ec);
	auto end = std::filesystem::recursive_directory_iterator();

	if (ec) return false;

	while (it != end) {
		if (it->is_regular_file(ec) && it->path().extension() == Project::projectFileExtension) {
			outPath = it->path();
			return true;
		}

		it.increment(ec);
		if (ec) {
			ec.clear();
			continue;
		}
	}

	return false;
}

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

	fs::path cache = rootPath / cacheDirectoryName;
	fs::create_directories(cache, ec);
	if (ec) {
		std::cerr << "[Error] Could not create " << cache << ": " << ec.message() << '\n';
		return false;
	}

	projectArchivePath = cache / (projectName + projectFileExtension);
	if (!fs::exists(projectArchivePath, ec)) {
		Archive arch;
		arch.set("name", projectName);
		arch.set("startupScene", std::string());

		if (!arch.saveToFile(projectArchivePath)) {
			std::cerr << "[Error] Could not write " << projectArchivePath.filename() << " in " << rootPath << '\n';
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

	bool archiveFound = tryGetProjectArchivePath(absolute, projectArchivePath);

	if (!archiveFound) return false;

	Archive arch;
	if (!arch.loadFromFile(projectArchivePath)) {
		std::cerr << "[Error] No readable " << projectFileExtension << " in " << absolute << '\n';
		return false;
	}

	std::string projectName = absolute.filename().string();
	std::string scene;
	arch.get("name", projectName);
	arch.get("startupScene", scene);

	root = std::move(absolute);
	cacheDirectory = root / cacheDirectoryName;
	name = std::move(projectName);
	startupScene = std::move(scene);
	projectOpen = true;

	copyComponentCache.reset();
	copyEntityCache.reset();
	copiedComponent = false;
	copiedEntity = false;

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

bool Project::isProjectFile(const fs::path& path) {
	return path == projectArchivePath;
}

bool Project::copyTransform(uint32_t entityID) {
	Scene* scene = Application::getCurrentScene();
	if (scene->copyTransform(entityID, copyComponentCache)) {
		copiedTransform = true;
		return copiedComponent = true;
	}
	return false;
}

bool Project::copyComponent(uint32_t entityID, uint32_t componentUID) {
	Scene* scene = Application::getCurrentScene();
	if (scene->copyComponent(entityID, componentUID, copyComponentCache)) {
		copiedTransform = false;
		return copiedComponent = true;
	}
	return false;
}

bool Project::copyEntity(uint32_t entityID) {
	Scene* scene = Application::getCurrentScene();
	if (scene->copyEntity(entityID, copyEntityCache)) {
		return copiedEntity = true;
	}
	return false;
}

void Project::pasteComponent(uint32_t entityID) {
	Scene* scene = Application::getCurrentScene();
	
	if (copiedTransform)
		scene->pasteTransform(entityID, copyComponentCache);
	else
		scene->pasteComponent(entityID, copyComponentCache);
}

void Project::pasteEntity() {
	Scene* scene = Application::getCurrentScene();
	scene->pasteEntity(copyEntityCache);
}

bool Project::hasCopiedComponent() {
	return copiedComponent;
}

bool Project::hasCopiedEntity() {
	return copiedEntity;
}

void Project::setStartupScene(const std::string& relativeScenePath) {
	if (!projectOpen || startupScene == relativeScenePath) return;

	startupScene = relativeScenePath;
	save();
}

std::mutex& Project::getSceneMutex() {
	return sceneMutex;
}

bool Project::save() {
	Archive arch;
	arch.set("name", name);
	arch.set("startupScene", startupScene);

	if (!arch.saveToFile(projectArchivePath)) {
		std::cerr << "[Error] Could not write " << projectArchivePath << '\n';
		return false;
	}

	return true;
}
