#pragma once

#include <filesystem>
#include <string>

namespace ProjectController {
	bool create(const std::filesystem::path& rootPath, const std::string& projectName);
	bool open(const std::filesystem::path& rootPath);
	bool isProjectOpen();
	bool tryGetProjectName(std::string& outName);
	const std::filesystem::path& getProjectRoot();
}