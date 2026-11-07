#include <controller/ProjectController.h>

#include <core/Project.h>

bool ProjectController::create(const std::filesystem::path& rootPath, const std::string& projectName) {
	return Project::create(rootPath, projectName);
}

bool ProjectController::open(const std::filesystem::path& rootPath) {
	return Project::open(rootPath);
}

bool ProjectController::isProjectOpen() {
	return Project::isOpen();
}

bool ProjectController::tryGetProjectName(std::string& outName) {
	outName = Project::getName();
	return !outName.empty();
}

const std::filesystem::path& ProjectController::getProjectRoot() {
	return Project::getRoot();
}