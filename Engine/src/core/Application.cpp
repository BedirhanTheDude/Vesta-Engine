#include <GLFW/glfw3.h>

#include <core/Application.h>
#include <core/Project.h>

#include <scene/Scene.h>
#include <renderer/Renderer.h>

std::unique_ptr<Scene> Application::m_Scene;
std::unique_ptr<Renderer> Application::m_Renderer;

GLFWwindow* Application::m_Window = nullptr;

Scene* Application::newScene(const std::string& relativeScenePath) {
	m_Scene = std::unique_ptr<Scene>(new Scene());

	if (Project::isOpen() && (relativeScenePath.empty() || !m_Scene->openScene(relativeScenePath)))
		Scene::createDefaultScene(*m_Scene);

	return m_Scene.get();
}

Scene* Application::getCurrentScene() {
	return m_Scene.get();
}

Renderer* Application::newRenderer() {
	m_Renderer = std::unique_ptr<Renderer>(new Renderer());
	return m_Renderer.get();
}

Renderer* Application::getCurrentRenderer() {
	return m_Renderer.get();
}

void Application::setWindow(GLFWwindow* w) {
	m_Window = w;
}

GLFWwindow* Application::getWindow() {
	return m_Window;
}

void Application::shutdown() {
	m_Scene.reset();
	m_Renderer.reset();
}