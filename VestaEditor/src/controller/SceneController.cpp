#include <controller/SceneController.h>
#include <persistance/SceneSerializer.h>

#include <scene/Scene.h>
#include <core/Application.h>
#include <core/Project.h>

#include <iostream>
#include <system_error>

bool SceneController::newScene(const std::string& name, const std::filesystem::path& directory) {
	std::filesystem::path file = directory / std::filesystem::u8path(name + Project::sceneExtension);

	std::string relativePath;
	if (!Project::toRelative(file, relativePath)) {
		std::cerr << "[Error] New scene " << file << " would be outside the project\n";
		return false;
	}

	std::error_code ec;
	if (std::filesystem::exists(file, ec)) {
		std::cerr << "[Error] " << file << " already exists\n";
		return false;
	}

	Scene* scene = getCurrentScene();
	scene->newScene(name);
	scene->setScenePath(relativePath);

	return true;
}

bool SceneController::openScene(const std::filesystem::path& sceneFile) {
	std::string relativePath;
	if (!Project::toRelative(sceneFile, relativePath)) {
		std::cerr << "[Error] " << sceneFile << " is not inside the project\n";
		return false;
	}

	return getCurrentScene()->openScene(relativePath);
}

void SceneController::loadScene(bool temp) {
	Scene* scene = getCurrentScene();

	SceneSerializer::load(*scene, temp);
}

void SceneController::saveScene(bool temp) {
	Scene* scene = getCurrentScene();

	if (!temp && scene->getScenePath().empty())
		scene->setScenePath(scene->getSceneName() + Project::sceneExtension);

	SceneSerializer::save(*scene, temp);
}

void SceneController::createEntity(const std::string& name) {
	Scene* scene = getCurrentScene();
	if (!scene) return;

	if (scene->isPlaying) scene->createEntity(name);
	else scene->createEntityImmediate(name);
}

void SceneController::removeEntity(const std::string& name) {
	Scene* scene = getCurrentScene();
	if (!scene) return;

	scene->removeEntity(name);
}

void SceneController::removeEntity(unsigned int ID) {
	Scene* scene = getCurrentScene();
	if (!scene) return;

	scene->removeEntity(ID);
}

void SceneController::playScene() {
	Scene* scene = getCurrentScene();
	if (!scene) return;

	saveScene(true);
	scene->isPlaying = true;
}

void SceneController::stopScene(bool interrupt) {
	Scene* scene = getCurrentScene();
	if (!scene) return;

	scene->isPlaying = false;
	loadScene(!interrupt); // if not interrupted load temp as usual
}

bool SceneController::sceneExists() {
	if (getCurrentScene()) return true;
	else return false;
}

bool SceneController::sceneIsPlaying() {
	Scene* scene = getCurrentScene();
	if (!scene)
		return false;
	else
		return scene->isPlaying;
}

Scene* SceneController::getCurrentScene() {
	return Application::getCurrentScene();
}