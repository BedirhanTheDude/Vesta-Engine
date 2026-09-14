#include <controller/FileTypeController.h>

#include <cstdlib>

bool FileTypeController::isImageFile(const std::filesystem::path& p) {
    auto e = p.extension().string();
    return e == ".png" || e == ".jpg" || e == ".jpeg" || e == ".bmp" || e == ".tga" || e == ".hdr";
}

bool FileTypeController::isAudioFile(const std::filesystem::path& p) {
    auto e = p.extension().string();
    return e == ".wav" || e == ".mp3" || e == ".ogg";
}

bool FileTypeController::isMeshFile(const std::filesystem::path& p) {
    auto e = p.extension().string();
    return e == ".obj" || e == ".fbx" || e == ".gltf" || e == ".glb";
}

bool FileTypeController::isSceneFile(const std::filesystem::path& p) {
    return p.extension().string() == ".scene" || p.extension().string() == ".vrea";
}

bool FileTypeController::isShaderFile(const std::filesystem::path& p) {
    auto e = p.extension().string();
    return e == ".glsl" || e == ".vert" || e == ".frag" || e == ".hlsl";
}

bool FileTypeController::isScriptFile(const std::filesystem::path& p) {
    auto e = p.extension().string();
    return e == ".lua" || e == ".py" || e == ".cs" || e == ".h" || e == ".cpp";
}

const char* FileTypeController::iconForExtension(const std::filesystem::path& p, bool isDirectory) {
    if (isDirectory)        return "[DIR]";
    if (isImageFile(p))     return "[IMG]";
    if (isMeshFile(p))      return "[OBJ]";
    if (isSceneFile(p))     return "[SCN]";
    if (isShaderFile(p))    return "[SHD]";
    if (isScriptFile(p))    return "[SCR]";
    if (isAudioFile(p))     return "[AUD]";
    return "[FILE]";
}

std::string FileTypeController::truncateForDisplay(const std::string& name, size_t maxChars) {
    if (name.size() <= maxChars) return name;
    if (maxChars < 3) return name.substr(0, maxChars);
    return name.substr(0, maxChars - 2) + "..";
}

void FileTypeController::revealInExplorer(const std::filesystem::path& target) {
#ifdef _WIN32
    std::string cmd = "explorer \"" + target.string() + "\"";
#elif __APPLE__
    std::string cmd = "open \"" + target.string() + "\"";
#else
    std::string cmd = "xdg-open \"" + target.string() + "\"";
#endif
    system(cmd.c_str());
}