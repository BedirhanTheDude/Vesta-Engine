#include <GLFW/glfw3.h>

#include <core/Application.h>
#include <core/Project.h>

#include <scene/Scene.h>
#include <renderer/Renderer.h>

std::unique_ptr<Scene> Application::scene;
std::unique_ptr<Renderer> Application::renderer;

GLFWwindow* Application::window = nullptr;

Scene* Application::newScene(const std::string& relativeScenePath) {
	scene = std::unique_ptr<Scene>(new Scene());

	if (Project::isOpen() && (relativeScenePath.empty() || !scene->openScene(relativeScenePath)))
		Scene::createDefaultScene(*scene);

	return scene.get();
}

Scene* Application::getCurrentScene() {
	return scene.get();
}

Renderer* Application::newRenderer() {
	renderer = std::unique_ptr<Renderer>(new Renderer());
	return renderer.get();
}

Renderer* Application::getCurrentRenderer() {
	return renderer.get();
}

void Application::setWindow(GLFWwindow* w) {
	window = w;
}

GLFWwindow* Application::getWindow() {
	return window;
}

void Application::shutdown() {
	scene.reset();
	renderer.reset();
}