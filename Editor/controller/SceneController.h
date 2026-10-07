#pragma once

#include <string>
#include <filesystem>

class Scene;

namespace SceneController {
	// the scene is saved to <directory>/<name>.vestascene on its first save,
	// false if the directory is outside the project or that file already exists
	bool newScene(const std::string& name, const std::filesystem::path& directory);

	// sceneFile is an absolute path to a scene file inside the project
	bool openScene(const std::filesystem::path& sceneFile);
	// reloads the current scene from its file (or the temp slot)
	void loadScene(bool temp = false);
	void saveScene(bool temp = false);

	void createEntity(const std::string& name = "New Entity");
	void removeEntity(const std::string& name);
	void removeEntity(unsigned int ID);

	void playScene();
	void stopScene(bool interrupt = false);

	bool sceneExists();
	bool sceneIsPlaying();

	Scene* getCurrentScene();
};
