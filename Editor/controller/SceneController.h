#pragma once

#include <string>

class Scene;

class SceneController {
public:
	static void newScene(const std::string& name = "Scene");

	static void loadScene(const std::string& name, bool temp = false);
	static void loadScene(bool temp = false);
	static void saveScene(bool temp = false);

	static void createEntity(const std::string& name = "New Entity");
	static void removeEntity(const std::string& name);
	static void removeEntity(unsigned int ID);

	static void playScene();
	static void stopScene();

	static bool sceneExists();
	static bool sceneIsPlaying();
private:
	static Scene* getCurrentScene();
};