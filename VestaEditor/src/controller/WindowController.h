#pragma once

#include <string>

struct GLFWwindow;

namespace WindowController {
	GLFWwindow* getApplicationWindow();
	void addWindowSuffix(const std::string& suffix);
	void removeWindowSuffix();
}