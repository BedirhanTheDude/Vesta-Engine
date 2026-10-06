#pragma once

#include <filesystem>
#include <string>
#include <mutex>

class Project {
public:
	static constexpr const char* projectFileExtension = ".vestaproj";
	static constexpr const char* sceneExtension = ".vestascene";
	static constexpr const char* cacheDirectoryName = ".vesta";

	// create a .vestaproj if not present
	static bool create(const std::filesystem::path& rootPath, const std::string& projectName);
	// fails if the directory has no .vestaproj
	static bool open(const std::filesystem::path& rootPath);

	static bool isOpen();

	static const std::filesystem::path& getRoot();
	static const std::filesystem::path& getCacheDirectory();
	static const std::string& getName();

	// false if no project is open or the path is empty, absolute or points outside the root ("../x")
	static bool resolve(const std::string& relativePath, std::filesystem::path& outPath);

	// false if it's outside the root
	static bool toRelative(const std::filesystem::path& path, std::string& outRelativePath);

	// directories that scans skip: the cache directory and any directory starting with '.'
	static bool isIgnoredDirectory(const std::filesystem::path& directory);
	static bool isProjectFile(const std::filesystem::path& path);

	static const std::string& getStartupScene();
	static void setStartupScene(const std::string& relativeScenePath);

	static std::mutex& getSceneMutex();

private:
	static bool save();

	static std::filesystem::path root;
	static std::filesystem::path cacheDirectory;
	static std::filesystem::path projectArchivePath;
	static std::string name;
	static std::string startupScene;

	static std::mutex sceneMutex;

	static bool projectOpen;
};
