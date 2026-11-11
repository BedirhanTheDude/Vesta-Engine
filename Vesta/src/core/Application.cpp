#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <stb_image.h>

#include <core/Application.h>
#include <core/Layer.h>
#include <core/Input.h>
#include <core/Project.h>

#include <scene/Scene.h>
#include <renderer/Renderer.h>

#include <iostream>

std::unique_ptr<Scene> Application::m_Scene;
std::unique_ptr<Renderer> Application::m_Renderer;

GLFWwindow* Application::m_Window = nullptr;

static void setWindowIcon(GLFWwindow* window, const std::string& path) {
	if (path.empty()) return;

	int width, height, channels;
	unsigned char* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
	if (!pixels) return;

	GLFWimage icon = { width, height, pixels };
	glfwSetWindowIcon(window, 1, &icon);
	stbi_image_free(pixels);
}

bool Application::init(const ApplicationSpecification& spec) {
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return false;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	m_Window = glfwCreateWindow(spec.width, spec.height, spec.title.c_str(), nullptr, nullptr);
	if (!m_Window) {
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return false;
	}

	setWindowIcon(m_Window, spec.iconPath);

	glfwMakeContextCurrent(m_Window);
	glfwSwapInterval(spec.vsync ? 1 : 0);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "Failed to initialize GLAD\n";
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
		glfwTerminate();
		return false;
	}

	Input::init(m_Window);

	// loads the project's startup scene if a project is open, otherwise the scene stays blank
	newScene(Project::getStartupScene());
	newRenderer();

	return true;
}

void Application::run(std::unique_ptr<Layer> layer) {
	if (!m_Window) return;

	if (layer) layer->onAttach();

	float last = static_cast<float>(glfwGetTime());

	while (!glfwWindowShouldClose(m_Window)) {
		float now = static_cast<float>(glfwGetTime());
		float dt = now - last;
		last = now;

		glfwPollEvents();
		Input::update();

		m_Scene->onUpdate(dt);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		if (layer) layer->onUpdate(dt);

		glfwSwapBuffers(m_Window);
	}

	if (layer) {
		layer->onDetach();
		layer.reset();
	}

	shutdown();
}

void Application::close() {
	if (m_Window) glfwSetWindowShouldClose(m_Window, true);
}

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

GLFWwindow* Application::getWindow() {
	return m_Window;
}

void Application::shutdown() {
	m_Scene.reset();
	m_Renderer.reset();

	if (m_Window) {
		glfwDestroyWindow(m_Window);
		m_Window = nullptr;
	}
	glfwTerminate();
}
