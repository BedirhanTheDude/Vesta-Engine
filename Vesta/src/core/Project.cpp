#include <core/Project.h>

#include <core/Application.h>
#include <persistence/Archive.h>

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

fs::path Project::m_Root;
fs::path Project::m_CacheDirectory;
fs::path Project::m_ProjectArchivePath;
std::string Project::m_Name;
std::string Project::m_StartupScene;
std::mutex Project::m_SceneMutex;
bool Project::m_ProjectOpen = false;

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

bool Project::isOpen() { return m_ProjectOpen; }
const fs::path& Project::getRoot() { return m_Root; }
const fs::path& Project::getCacheDirectory() { return m_CacheDirectory; }
const std::string& Project::getName() { return m_Name; }
const std::string& Project::getStartupScene() { return m_StartupScene; }

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

	m_ProjectArchivePath = cache / (projectName + projectFileExtension);
	if (!fs::exists(m_ProjectArchivePath, ec)) {
		Archive arch;
		arch.set("name", projectName);
		arch.set("startupScene", std::string());

		if (!arch.saveToFile(m_ProjectArchivePath)) {
			std::cerr << "[Error] Could not write " << m_ProjectArchivePath.filename() << " in " << rootPath << '\n';
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

	bool archiveFound = tryGetProjectArchivePath(absolute, m_ProjectArchivePath);

	if (!archiveFound) return false;

	Archive arch;
	if (!arch.loadFromFile(m_ProjectArchivePath)) {
		std::cerr << "[Error] No readable " << projectFileExtension << " in " << absolute << '\n';
		return false;
	}

	std::string projectName = absolute.filename().string();
	std::string scene;
	arch.get("name", projectName);
	arch.get("startupScene", scene);

	m_Root = std::move(absolute);
	m_CacheDirectory = m_Root / cacheDirectoryName;
	m_Name = std::move(projectName);
	m_StartupScene = std::move(scene);
	m_ProjectOpen = true;

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
	if (!m_ProjectOpen || relativePath.empty()) return false;

	fs::path relative = fs::u8path(relativePath).lexically_normal();

	// a scene file is not allowed to reach outside of the project
	if (relative.has_root_name() || relative.has_root_directory()) return false;
	if (relative.empty() || *relative.begin() == "..") return false;

	outPath = m_Root / relative;
	return true;
}

bool Project::toRelative(const fs::path& path, std::string& outRelativePath) {
	if (!m_ProjectOpen) return false;

	std::error_code ec;
	fs::path absolute = fs::absolute(path, ec);
	if (ec) return false;

	fs::path relative = absolute.lexically_normal().lexically_relative(m_Root);
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
	return path == m_ProjectArchivePath;
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
	if (!m_ProjectOpen || m_StartupScene == relativeScenePath) return;

	m_StartupScene = relativeScenePath;
	save();
}

std::mutex& Project::getSceneMutex() {
	return m_SceneMutex;
}

bool Project::save() {
	Archive arch;
	arch.set("name", m_Name);
	arch.set("startupScene", m_StartupScene);

	if (!arch.saveToFile(m_ProjectArchivePath)) {
		std::cerr << "[Error] Could not write " << m_ProjectArchivePath << '\n';
		return false;
	}

	return true;
}
