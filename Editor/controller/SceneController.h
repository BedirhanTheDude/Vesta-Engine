#pragma once

#include <string>

class Scene;

namespace SceneController {
	void newScene(const std::string& name = "Scene");

	void loadScene(const std::string& name, bool temp = false);
	void loadScene(bool temp = false);
	void saveScene(bool temp = false);

	void createEntity(const std::string& name = "New Entity");
	void removeEntity(const std::string& name);
	void removeEntity(unsigned int ID);

	void playScene();
	void stopScene();

	bool sceneExists();
	bool sceneIsPlaying();

	Scene* getCurrentScene();
};