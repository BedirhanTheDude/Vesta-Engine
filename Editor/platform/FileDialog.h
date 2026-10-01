#pragma once

#include <filesystem>

struct GLFWwindow;

namespace FileDialog {
	bool pickFolder(GLFWwindow* owner, std::filesystem::path& outFolder);
}