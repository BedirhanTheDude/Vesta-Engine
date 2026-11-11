#pragma once

#include <string>
#include <memory>

class Scene;
class Renderer;
class Layer;

struct GLFWwindow;

struct ApplicationSpecification {
	std::string title = "Vesta Engine";
	int width = 1000;
	int height = 800;
	std::string iconPath = "icon.png"; // relative to the working directory, skipped if missing
	bool vsync = false;
};

class Application {
public:
	static bool init(const ApplicationSpecification& spec = {});
	static void run(std::unique_ptr<Layer> layer);
	static void close();

	static Scene* newScene(const std::string& relativeScenePath);
	static Scene* getCurrentScene();

	static Renderer* newRenderer();
	static Renderer* getCurrentRenderer();

	static GLFWwindow* getWindow();

private:
	static void shutdown();

	static std::unique_ptr<Scene> m_Scene;
	static std::unique_ptr<Renderer> m_Renderer;
	static GLFWwindow* m_Window;
};
