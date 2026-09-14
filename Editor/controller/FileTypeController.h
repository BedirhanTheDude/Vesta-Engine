#pragma once

#include <string>
#include <filesystem>

class FileTypeController {
public:
    static bool isImageFile(const std::filesystem::path& p);
    static bool isAudioFile(const std::filesystem::path& p);
    static bool isMeshFile(const std::filesystem::path& p);
    static bool isSceneFile(const std::filesystem::path& p);
    static bool isShaderFile(const std::filesystem::path& p);
    static bool isScriptFile(const std::filesystem::path& p);

    // "[DIR]" / "[IMG]" / "[OBJ]" / "[SCN]" / "[SHD]" / "[SCR]" / "[AUD]" / "[FILE]"
    static const char* iconForExtension(const std::filesystem::path& p, bool isDirectory);

    static std::string truncateForDisplay(const std::string& name, size_t maxChars = 14);

    static void revealInExplorer(const std::filesystem::path& target);
};