#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>

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

    const std::filesystem::path& getCurrentDirectory() const { return m_CurrentDir; }
    const std::vector<std::string>& getScriptNames() const { return m_ScriptNames; }

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

    std::filesystem::path m_RootPath;
    std::filesystem::path m_CurrentDir;

    FileNode m_RootNode;
    std::vector<FileNode> m_CurrentItems;

    bool m_RenamingActive = false;
    std::filesystem::path m_RenamingPath;
    char m_RenameBuffer[256] = {};

    bool m_DeletePopupTriggered = false;
    FileNode* m_NodeToBeDeleted = nullptr;

    bool m_CreateFolderOpen = false;
    char m_NewFolderName[128] = "New Folder";

    bool m_ImportPopupOpen = false;
    std::filesystem::path m_SelectedPath;

    float m_ItemSize = 80.0f;
    float m_RefreshTimer = 0.0f;
    float m_RefreshInterval = 2.0f;

    std::filesystem::path m_PendingNavigate;

    enum class Tab { Assets, Scripts };
    Tab m_ActiveTab = Tab::Assets;

    std::vector<FileNode> m_ScriptItems;
    std::vector<std::string> m_ScriptNames;

    void refreshScripts();
    void drawScriptsTab();

    bool m_ImportScriptPopupOpen = false;
    static char m_ImportScriptPathBuf[512];
};