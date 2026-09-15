#include <controller/SceneController.h>
#include <persistance/SceneSerializer.h>

#include <scene/Scene.h>
#include <core/Application.h>

void SceneController::newScene(const std::string& name) {
	Scene* scene = getCurrentScene();
	scene->renameScene(name);
	scene->clear();

	Scene::createDefaultScene(*scene);
}

void SceneController::loadScene(const std::string& name, bool temp) {
	Scene* scene = getCurrentScene();
	scene->renameScene(name);
	scene->clear();

	SceneSerializer::load(*scene, temp);
}

void SceneController::loadScene(bool temp) {
	Scene* scene = getCurrentScene();
	scene->clear();

	SceneSerializer::load(*scene, temp);
}

void SceneController::saveScene(bool temp) {
	Scene* scene = getCurrentScene();
	
	SceneSerializer::save(*scene, temp);
}

void SceneController::createEntity(const std::string& name) {
	Scene* scene = getCurrentScene();
	if (scene->isPlaying) scene->createEntity(name);
	else scene->createEntityImmediate(name);
}

void SceneController::removeEntity(const std::string& name) {
	Scene* scene = getCurrentScene();
	scene->removeEntity(name);
}

void SceneController::removeEntity(unsigned int ID) {
	Scene* scene = getCurrentScene();
	scene->removeEntity(ID);
}

void SceneController::playScene() {
	Scene* scene = getCurrentScene();

	saveScene(true);
	scene->isPlaying = true;
}

void SceneController::stopScene() {
	Scene* scene = getCurrentScene();

	scene->isPlaying = false;
	loadScene(scene->getSceneName(), true);
}

bool SceneController::sceneExists() {
	if (getCurrentScene()) return true;
	else return false;
}

bool SceneController::sceneIsPlaying() {
	return getCurrentScene()->isPlaying;
}

Scene* SceneController::getCurrentScene() {
	return Application::getCurrentScene();
}