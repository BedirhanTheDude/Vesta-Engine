#pragma once

#include <string>
#include <memory>

class Scene;
class Renderer;

class Application {
public:
	static Scene* newScene(const std::string& name);
	static Scene* getCurrentScene();

	static Renderer* newRenderer();
	static Renderer* getCurrentRenderer();

	static void shutdown();

private:
	static std::unique_ptr<Scene> scene;
	static std::unique_ptr<Renderer> renderer;
};