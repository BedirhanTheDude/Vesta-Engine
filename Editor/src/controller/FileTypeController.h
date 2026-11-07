#pragma once

#include <string>
#include <filesystem>

namespace FileTypeController {
    bool isImageFile(const std::filesystem::path& p);
    bool isAudioFile(const std::filesystem::path& p);
    bool isMeshFile(const std::filesystem::path& p);
    bool isSceneFile(const std::filesystem::path& p);
    bool isShaderFile(const std::filesystem::path& p);
    bool isScriptFile(const std::filesystem::path& p);

    // the project cache (.vesta) and other dot-directories, never shown or scanned
    bool isHiddenDirectory(const std::filesystem::path& p);
    bool isProjectFile(const std::filesystem::path& p);

    // "[DIR]" / "[IMG]" / "[OBJ]" / "[SCN]" / "[SHD]" / "[SCR]" / "[AUD]" / "[FILE]"
    const char* iconForExtension(const std::filesystem::path& p, bool isDirectory);

    std::string truncateForDisplay(const std::string& name, size_t maxChars = 14);

    void revealInExplorer(const std::filesystem::path& target);
};