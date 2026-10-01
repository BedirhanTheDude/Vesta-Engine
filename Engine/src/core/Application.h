#pragma once

#include <string>
#include <memory>

class Scene;
class Renderer;

struct GLFWwindow;

class Application {
public:
	static Scene* newScene(const std::string& relativeScenePath);
	static Scene* getCurrentScene();

	static Renderer* newRenderer();
	static Renderer* getCurrentRenderer();

	static void setWindow(GLFWwindow* w);
	static GLFWwindow* getWindow();

	static void shutdown();

private:
	static std::unique_ptr<Scene> scene;
	static std::unique_ptr<Renderer> renderer;
	static GLFWwindow* window;
};