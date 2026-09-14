#include <gui/FileManagerPanel.h>

#include <controller/SceneController.h>
#include <controller/AssetController.h>
#include <controller/FileTypeController.h>

#include <imgui.h>

#include <glad/glad.h>
#include <stb_image.h>

#include <algorithm>
#include <iostream>
#include <cstring>

void FileManagerPanel::init(const std::string& rootPath) {
    this->rootPath = std::filesystem::absolute(rootPath);
    currentDir = this->rootPath;

    if (!std::filesystem::exists(this->rootPath))
        std::filesystem::create_directories(this->rootPath);

    refresh();
}

void FileManagerPanel::refresh() {
    rootNode = buildTree(rootPath);
    currentItems.clear();
    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(currentDir, ec)) {
        FileNode n;
        n.path = entry.path();
        n.name = entry.path().filename().string();
        n.isDirectory = entry.is_directory();
        currentItems.push_back(n);
    }
    std::sort(currentItems.begin(), currentItems.end(),
        [](const FileNode& a, const FileNode& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.name < b.name;
        });
}

FileNode FileManagerPanel::buildTree(const std::filesystem::path& path, int depth) const {
    FileNode node;
    node.path = path;
    node.name = path.filename().string();
    node.isDirectory = std::filesystem::is_directory(path);

    if (node.isDirectory && depth < 8) {
        std::error_code ec;
        for (auto& entry : std::filesystem::directory_iterator(path, ec)) {
            if (entry.is_directory())
                node.children.push_back(
                    std::make_shared<FileNode>(buildTree(entry.path(), depth + 1)));
        }
        std::sort(node.children.begin(), node.children.end(),
            [](const std::shared_ptr<FileNode>& a, const std::shared_ptr<FileNode>& b)
            { return a->name < b->name; });
    }
    return node;
}

unsigned int FileManagerPanel::loadThumbnail(const std::filesystem::path& imagePath) {
    std::string key = imagePath.string();
    auto it = thumbnailCache.find(key);
    if (it != thumbnailCache.end()) return it->second;

    int w, h, ch;
    stbi_set_flip_vertically_on_load(false);
    unsigned char* data = stbi_load(key.c_str(), &w, &h, &ch, 4);
    if (!data) return 0;

    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(data);
    thumbnailCache[key] = texID;
    return texID;
}

void FileManagerPanel::releaseThumbnails() {
    for (auto& [k, id] : thumbnailCache)
        if (id) glDeleteTextures(1, &id);
    thumbnailCache.clear();
}

void FileManagerPanel::createFolder(const std::filesystem::path& parent, const std::string& name) {
    std::filesystem::path newDir = parent / name;
    std::error_code ec;
    std::filesystem::create_directory(newDir, ec);
    if (ec) std::cerr << "Create folder failed: " << ec.message() << "\n";
    refresh();
}

void FileManagerPanel::renameItem(FileNode& node, const std::string& newName) {
    if (newName.empty() || newName == node.name) return;

    std::filesystem::path dest = node.path.parent_path() / newName;
    std::error_code ec;
    std::filesystem::rename(node.path, dest, ec);
    if (ec) std::cerr << "Rename failed: " << ec.message() << "\n";

    selectedPath = dest;
    refresh();
}

void FileManagerPanel::deleteItem(FileNode& node) {
    std::error_code ec;
    if (node.isDirectory)
        std::filesystem::remove_all(node.path, ec);
    else
        std::filesystem::remove(node.path, ec);

    if (ec) std::cerr << "Delete failed: " << ec.message() << "\n";

    if (selectedPath == node.path)
        selectedPath.clear();

    refresh();
}

void FileManagerPanel::importAsset(const std::string& srcAbsolutePath) {
    std::filesystem::path src(srcAbsolutePath);
    if (!std::filesystem::exists(src)) return;

    std::filesystem::path dest = currentDir / src.filename();
    std::error_code ec;
    std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) std::cerr << "Import copy failed: " << ec.message() << "\n";

    refresh();
}

char FileManagerPanel::importScriptPathBuf[512] = {};

void FileManagerPanel::initScriptsPath(const std::string& path) {
    scriptsPath = std::filesystem::absolute(path);
    if (!std::filesystem::exists(scriptsPath))
        std::filesystem::create_directories(scriptsPath);
    refreshScripts();
}

void FileManagerPanel::refreshScripts() {
    scriptItems.clear();
    if (scriptsPath.empty() || !std::filesystem::exists(scriptsPath)) return;
    std::error_code ec;
    for (auto& entry : std::filesystem::directory_iterator(scriptsPath, ec)) {
        if (!entry.is_regular_file()) continue;
        auto ext = entry.path().extension().string();
        if (ext != ".h" && ext != ".cpp" && ext != ".lua" && ext != ".py" && ext != ".cs") continue;
        FileNode n;
        n.path = entry.path();
        n.name = entry.path().filename().string();
        n.isDirectory = false;
        scriptItems.push_back(n);
    }
    std::sort(scriptItems.begin(), scriptItems.end(),
        [](const FileNode& a, const FileNode& b) { return a.name < b.name; });
}

std::vector<std::string> FileManagerPanel::getScriptNames() const {
    std::vector<std::string> names;
    for (auto& item : scriptItems)
        names.push_back(item.path.filename().string());   // == stem + extension, simpler
    return names;
}

void FileManagerPanel::importScript(const std::string& srcAbsolutePath) {
    std::filesystem::path src(srcAbsolutePath);
    if (!std::filesystem::exists(src)) return;
    std::filesystem::path dest = scriptsPath / src.filename();
    std::error_code ec;
    std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) std::cerr << "Script import failed: " << ec.message() << "\n";
    refreshScripts();
}

void FileManagerPanel::drawScriptsTab() {
    if (ImGui::Button("Add Script")) importScriptPopupOpen = true;
    ImGui::SameLine();
    if (ImGui::Button("Refresh##scripts")) refreshScripts();
    ImGui::Separator();

    if (scriptItems.empty()) {
        ImGui::TextDisabled("No scripts found in scripts folder.");
        ImGui::TextDisabled("Click 'Add Script' to import a .h/.cpp file.");
    }

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / (itemSize + 16.0f)));
    ImGui::Columns(columns, "##scriptgrid", false);

    for (auto& node : scriptItems) {
        ImGui::PushID(node.path.string().c_str());

        bool selected = (selectedPath == node.path);
        if (selected) ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.26f, 0.59f, 0.98f, 0.35f));

        ImGui::Button("[SCR]", ImVec2(itemSize, itemSize));

        if (selected) ImGui::PopStyleColor();

        if (ImGui::IsItemClicked()) selectedPath = node.path;

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            std::string pathStr = node.path.string();
            ImGui::SetDragDropPayload(scriptPayload, pathStr.c_str(), pathStr.size() + 1);
            ImGui::Text("[SCR] %s", node.name.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginPopupContextItem("##scrctx")) {
            ImGui::TextDisabled("%s", node.name.c_str());
            ImGui::Separator();
            if (ImGui::MenuItem("Show in Explorer")) {
                FileTypeController::revealInExplorer(scriptsPath);
            }
            if (ImGui::MenuItem("Delete")) {
                std::error_code ec;
                std::filesystem::remove(node.path, ec);
                refreshScripts();
                ImGui::EndPopup();
                ImGui::PopID();
                ImGui::Columns(1);
                return;
            }
            ImGui::EndPopup();
        }

        std::string display = FileTypeController::truncateForDisplay(node.name);
        float textWidth = ImGui::CalcTextSize(display.c_str()).x;
        float indent = (itemSize - textWidth) * 0.5f;
        if (indent > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        ImGui::TextUnformatted(display.c_str());
        ImGui::Spacing();

        ImGui::PopID();
        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

void FileManagerPanel::show(bool* open) {
    ImGuiIO& io = ImGui::GetIO();
    refreshTimer += io.DeltaTime;
    if (refreshTimer >= refreshInterval) {
        refreshTimer = 0.0f;
        refresh();
        refreshScripts();
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (!ImGui::Begin("File Manager", open, flags)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##FMTabs")) {
        if (ImGui::BeginTabItem("Assets")) {
            activeTab = Tab::Assets;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scripts")) {
            activeTab = Tab::Scripts;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    if (activeTab == Tab::Scripts) {
        ImGui::BeginChild("##FMScripts", ImVec2(0, 0), false,
            ImGuiWindowFlags_HorizontalScrollbar);
        drawScriptsTab();
        ImGui::EndChild();

        if (importScriptPopupOpen) {
            ImGui::OpenPopup("Import Script##FM");
            importScriptPopupOpen = false;
        }
        if (ImGui::BeginPopupModal("Import Script##FM", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Paste absolute path to script file (.h/.cpp/.lua):");
            ImGui::InputText("##importscript", importScriptPathBuf, sizeof(importScriptPathBuf));
            if (ImGui::Button("Import", ImVec2(120, 0))) {
                importScript(std::string(importScriptPathBuf));
                memset(importScriptPathBuf, 0, sizeof(importScriptPathBuf));
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::End();
        return;
    }

    if (ImGui::Button("Import")) importPopupOpen = true;
    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        memset(newFolderName, 0, sizeof(newFolderName));
        strcpy(newFolderName, "New Folder");
        createFolderOpen = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) refresh();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("Size", &itemSize, 48.0f, 128.0f);

    ImGui::Separator();
    drawBreadcrumb();
    ImGui::Separator();

    float treeWidth = 200.0f;
    ImGui::BeginChild("##FMTree", ImVec2(treeWidth, 0), true);
    drawFolderTree(rootNode);
    ImGui::EndChild();

    if (!pendingNavigate.empty()) {
        currentDir = pendingNavigate;
        pendingNavigate.clear();
        refresh();
    }

    ImGui::SameLine();

    ImGui::BeginChild("##FMContent", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar);
    drawContentBrowser();
    ImGui::EndChild();

    if (createFolderOpen) {
        ImGui::OpenPopup("Create Folder##FM");
        createFolderOpen = false;
    }
    if (ImGui::BeginPopupModal("Create Folder##FM", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Folder name:");
        ImGui::InputText("##foldername", newFolderName, sizeof(newFolderName));
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            createFolder(currentDir, newFolderName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (importPopupOpen) {
        ImGui::OpenPopup("Import Asset##FM");
        importPopupOpen = false;
    }
    static char importPath[512] = {};
    if (ImGui::BeginPopupModal("Import Asset##FM", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Paste absolute file path to import:");
        ImGui::InputText("##importpath", importPath, sizeof(importPath));
        if (ImGui::Button("Import", ImVec2(120, 0))) {
            importAsset(std::string(importPath));
            memset(importPath, 0, sizeof(importPath));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    ImGui::End();
}

void FileManagerPanel::drawBreadcrumb() {
    std::vector<std::filesystem::path> crumbs;
    std::filesystem::path cur = currentDir;
    while (cur != rootPath.parent_path() && cur != cur.parent_path()) {
        crumbs.push_back(cur);
        if (cur == rootPath) break;
        cur = cur.parent_path();
    }
    std::reverse(crumbs.begin(), crumbs.end());

    for (size_t i = 0; i < crumbs.size(); ++i) {
        if (i > 0) {
            ImGui::SameLine();
            ImGui::Text("/");
            ImGui::SameLine();
        }
        std::string label = crumbs[i].filename().string() + "##bc" + std::to_string(i);
        if (ImGui::SmallButton(label.c_str())) {
            currentDir = crumbs[i];
            refresh();
        }
    }
}

void FileManagerPanel::drawFolderTree(FileNode& node) {
    if (!node.isDirectory) return;

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (node.children.empty())
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (node.path == currentDir)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (node.path == rootPath)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    std::string label = "[DIR] " + node.name + "##tree" + node.path.string();
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        pendingNavigate = node.path;
    }

    if (open) {
        for (auto& child : node.children)
            drawFolderTree(*child);
        ImGui::TreePop();
    }
}

void FileManagerPanel::drawContentBrowser() {
    drawContextMenuBackground();

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* osPayload = ImGui::AcceptDragDropPayload("_NATIVE_FILE")) {
            std::string droppedPath(static_cast<const char*>(osPayload->Data),
                osPayload->DataSize);
            importAsset(droppedPath);
        }
        ImGui::EndDragDropTarget();
    }

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / (itemSize + 16.0f)));

    ImGui::Columns(columns, "##fmgrid", false);

    for (auto& node : currentItems) {
        drawItem(node);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void FileManagerPanel::drawItem(FileNode& node) {
    bool selected = (selectedPath == node.path);

    ImGui::PushID(node.path.string().c_str());

    if (renamingActive && renamingPath == node.path) {
        ImGui::SetNextItemWidth(itemSize);
        if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_AutoSelectAll)) {
            renameItem(node, std::string(renameBuffer));
            renamingActive = false;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            renamingActive = false;
        }
        ImGui::PopID();
        return;
    }

    ImVec2 iconSize(itemSize, itemSize);

    if (selected) {
        ImVec4 selColor(0.26f, 0.59f, 0.98f, 0.35f);
        ImGui::PushStyleColor(ImGuiCol_Button, selColor);
    }

    bool clicked = false;
    if (FileTypeController::isImageFile(node.path)) {
        unsigned int thumb = loadThumbnail(node.path);
        if (thumb) {
            clicked = ImGui::ImageButton(
                ("##btn" + node.path.string()).c_str(),
                (ImTextureID)(intptr_t)thumb,
                iconSize);
        }
        else {
            clicked = ImGui::Button(
                FileTypeController::iconForExtension(node.path, node.isDirectory),
                iconSize);
        }
    }
    else {
        clicked = ImGui::Button(
            FileTypeController::iconForExtension(node.path, node.isDirectory),
            iconSize);
    }

    if (selected) ImGui::PopStyleColor();

    if (clicked || ImGui::IsItemClicked()) {
        selectedPath = node.path;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (node.isDirectory) {
            pendingNavigate = node.path;
        }
        else if (FileTypeController::isSceneFile(node.path)) {
            SceneController::loadScene(node.path.stem().string());
        }
    }

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        std::string pathStr = node.path.string();
        ImGui::SetDragDropPayload(payload, pathStr.c_str(), pathStr.size() + 1);
        ImGui::Text("%s %s", FileTypeController::iconForExtension(node.path, node.isDirectory), node.name.c_str());
        ImGui::EndDragDropSource();
    }

    drawContextMenuItem(node);

    std::string display = FileTypeController::truncateForDisplay(node.name);
    float textWidth = ImGui::CalcTextSize(display.c_str()).x;
    float indent = (itemSize - textWidth) * 0.5f;
    if (indent > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
    ImGui::TextUnformatted(display.c_str());

    ImGui::Spacing();
    ImGui::PopID();
}

void FileManagerPanel::drawContextMenuBackground() {
    if (ImGui::BeginPopupContextWindow("##FMBackgroundCtx",
        ImGuiPopupFlags_MouseButtonRight |
        ImGuiPopupFlags_NoOpenOverItems)) {
        if (ImGui::MenuItem("New Folder")) {
            memset(newFolderName, 0, sizeof(newFolderName));
            strcpy(newFolderName, "New Folder");
            createFolderOpen = true;
        }
        if (ImGui::MenuItem("Import Asset...")) importPopupOpen = true;
        if (ImGui::MenuItem("Refresh"))         refresh();
        ImGui::EndPopup();
    }
}

void FileManagerPanel::drawContextMenuItem(FileNode& node) {
    std::string popupID = "##FMItemCtx" + node.path.string();

    if (ImGui::BeginPopupContextItem(popupID.c_str())) {
        ImGui::TextDisabled("%s", node.name.c_str());
        ImGui::Separator();

        if (node.isDirectory) {
            if (ImGui::MenuItem("Open")) {
                pendingNavigate = node.path;
            }
        }
        else if (FileTypeController::isSceneFile(node.path)) {
            if (ImGui::MenuItem("Open Scene"))
                SceneController::loadScene(node.path.stem().string());
        }
        else if (FileTypeController::isMeshFile(node.path)) {
            if (ImGui::MenuItem("Load Mesh")) {
                AssetController::loadMesh(node.name);
            }
        }
        else if (FileTypeController::isImageFile(node.path)) {
            if (ImGui::MenuItem("Load Texture")) {
                AssetController::loadTexture(node.name);
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Rename")) {
            renamingActive = true;
            renamingPath = node.path;
            std::string stem = node.isDirectory
                ? node.name
                : node.path.stem().string();
            strncpy(renameBuffer, stem.c_str(), sizeof(renameBuffer) - 1);
        }

        if (ImGui::MenuItem("Show in Explorer")) {
            std::filesystem::path target = node.isDirectory ? node.path : node.path.parent_path();
            FileTypeController::revealInExplorer(target);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete")) {
            ImGui::OpenPopup(("Delete?##" + node.path.string()).c_str());
        }
        if (ImGui::BeginPopupModal(("Delete?##" + node.path.string()).c_str(),
            nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Delete \"%s\"?\nThis cannot be undone.", node.name.c_str());
            ImGui::Separator();
            if (ImGui::Button("Delete", ImVec2(120, 0))) {
                deleteItem(node);
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                ImGui::EndPopup();
                return;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}