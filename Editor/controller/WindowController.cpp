#include <GLFW/glfw3.h>

#include <controller/WindowController.h>

#include <core/Application.h>

#include <string>
#include <format>

static constexpr const char* baseTitle = "Vesta Engine";

GLFWwindow* WindowController::getApplicationWindow() {
	return Application::getWindow();
}

void WindowController::addWindowSuffix(const std::string& suffix) {
	std::string newTitle = baseTitle;
	newTitle += " - ";
	newTitle += suffix;

	glfwSetWindowTitle(WindowController::getApplicationWindow(), newTitle.c_str());
}

void WindowController::removeWindowSuffix() {
	glfwSetWindowTitle(WindowController::getApplicationWindow(), baseTitle);
}