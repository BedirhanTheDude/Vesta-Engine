#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <unordered_map>

struct FileNode {
    std::string name;
    std::filesystem::path path;
    bool isDirectory = false;
    std::vector<std::shared_ptr<FileNode>> children;
    unsigned int previewTexture = 0;
};

class FileManagerPanel {
public:
    // rootPath is the project root
    void init(const std::filesystem::path& rootPath);
    void show(bool* open = nullptr);

    static constexpr const char* payload = "FM_ASSET_PATH";
    static constexpr const char* scriptPayload = "FM_SCRIPT_PATH";

    const std::filesystem::path& getCurrentDirectory() const { return currentDir; }
    const std::vector<std::string>& getScriptNames() const { return scriptNames; }

private:
    void drawFolderTree(FileNode& node);
    void drawContentBrowser();
    void drawBreadcrumb();
    void drawItem(FileNode& node);
    void drawContextMenuBackground();
    void drawContextMenuItem(FileNode& node);

    void refresh();
    FileNode buildTree(const std::filesystem::path& path, int depth = 0) const;

    void createFolder(const std::filesystem::path& parent, const std::string& name);
    void renameItem(FileNode& node, const std::string& newName);
    void deleteItem(FileNode& node);
    void importAsset(const std::string& srcAbsolutePath);

    unsigned int loadThumbnail(const std::filesystem::path& imagePath);
    void releaseThumbnails();

    std::filesystem::path rootPath;
    std::filesystem::path currentDir;

    FileNode rootNode;
    std::vector<FileNode> currentItems;

    bool renamingActive = false;
    std::filesystem::path renamingPath;
    char renameBuffer[256] = {};

    bool deletePopupTriggered = false;
    FileNode* nodeToBeDeleted = nullptr;

    bool createFolderOpen = false;
    char newFolderName[128] = "New Folder";

    bool importPopupOpen = false;
    std::filesystem::path selectedPath;
    std::unordered_map<std::string, unsigned int> thumbnailCache;

    float itemSize = 80.0f;
    float refreshTimer = 0.0f;
    float refreshInterval = 2.0f;

    std::filesystem::path pendingNavigate;

    enum class Tab { Assets, Scripts };
    Tab activeTab = Tab::Assets;

    std::vector<FileNode> scriptItems;
    std::vector<std::string> scriptNames;

    void refreshScripts();
    void drawScriptsTab();

    bool importScriptPopupOpen = false;
    static char importScriptPathBuf[512];
};