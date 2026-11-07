#include <gui/FileManagerPanel.h>

#include <controller/ScriptingController.h>
#include <controller/SceneController.h>
#include <controller/AssetController.h>
#include <controller/FileTypeController.h>

#include <gui/Thumbnail.h>

#include <imgui.h>

#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdint>

void FileManagerPanel::init(const std::filesystem::path& rootPath) {
    this->m_RootPath = rootPath;
    m_CurrentDir = this->m_RootPath;

    refresh();
    refreshScripts();
}

void FileManagerPanel::refresh() {
    m_RenamingActive = false;
    m_DeletePopupTriggered = false;
    m_CreateFolderOpen = false;
    m_ImportPopupOpen = false;
    m_ImportScriptPopupOpen = false;
    m_NodeToBeDeleted = nullptr;

    // the current directory may have been deleted or renamed from outside
    std::error_code ec;
    if (!std::filesystem::is_directory(m_CurrentDir, ec))
        m_CurrentDir = m_RootPath;

    m_RootNode = buildTree(m_RootPath);
    m_CurrentItems.clear();
    for (auto& entry : std::filesystem::directory_iterator(m_CurrentDir, ec)) {
        std::error_code entryEc;
        FileNode n;
        n.path = entry.path();
        n.name = entry.path().filename().string();
        n.isDirectory = entry.is_directory(entryEc);

        if (n.isDirectory && FileTypeController::isHiddenDirectory(n.path)) continue;
        if (!n.isDirectory && FileTypeController::isProjectFile(n.path)) continue;

        m_CurrentItems.push_back(n);
    }
    std::sort(m_CurrentItems.begin(), m_CurrentItems.end(),
        [](const FileNode& a, const FileNode& b) {
            if (a.isDirectory != b.isDirectory) return a.isDirectory > b.isDirectory;
            return a.name < b.name;
        });
}

FileNode FileManagerPanel::buildTree(const std::filesystem::path& path, int depth) const {
    FileNode node;
    node.path = path;
    node.name = path.filename().string();

    std::error_code ec;
    node.isDirectory = std::filesystem::is_directory(path, ec);

    if (node.isDirectory && depth < 8) {
        for (auto& entry : std::filesystem::directory_iterator(path, ec)) {
            std::error_code entryEc;
            if (entry.is_directory(entryEc) && !FileTypeController::isHiddenDirectory(entry.path()))
                node.children.push_back(
                    std::make_shared<FileNode>(buildTree(entry.path(), depth + 1)));
        }
        std::sort(node.children.begin(), node.children.end(),
            [](const std::shared_ptr<FileNode>& a, const std::shared_ptr<FileNode>& b)
            { return a->name < b->name; });
    }
    return node;
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

    m_SelectedPath = dest;
    refresh();
}

void FileManagerPanel::deleteItem(FileNode& node) {
    std::error_code ec;
    if (node.isDirectory)
        std::filesystem::remove_all(node.path, ec);
    else
        std::filesystem::remove(node.path, ec);

    if (ec) std::cerr << "Delete failed: " << ec.message() << "\n";

    if (m_SelectedPath == node.path)
        m_SelectedPath.clear();

    refresh();
}

void FileManagerPanel::importAsset(const std::string& srcAbsolutePath) {
    std::filesystem::path src = std::filesystem::u8path(srcAbsolutePath);
    std::error_code existsEc;
    if (!std::filesystem::exists(src, existsEc)) return;

    std::filesystem::path dest = m_CurrentDir / src.filename();
    std::error_code ec;
    std::filesystem::copy_file(src, dest, std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) std::cerr << "Import copy failed: " << ec.message() << "\n";

    refresh();
}

char FileManagerPanel::m_ImportScriptPathBuf[512] = {};

// scripts can be anywhere in the project, this walks the whole tree (skipping hidden directories)
void FileManagerPanel::refreshScripts() {
    m_ScriptItems.clear();
    m_ScriptNames.clear();

    std::error_code ec;
    std::filesystem::recursive_directory_iterator it(m_RootPath,
        std::filesystem::directory_options::skip_permission_denied, ec);
    const std::filesystem::recursive_directory_iterator end;

    for (; !ec && it != end; it.increment(ec)) {
        const std::filesystem::directory_entry& entry = *it;
        std::error_code entryEc;

        if (entry.is_directory(entryEc)) {
            if (FileTypeController::isHiddenDirectory(entry.path()))
                it.disable_recursion_pending();
            continue;
        }

        if (!entry.is_regular_file(entryEc)) continue;

        const std::filesystem::path extension = entry.path().extension();
        if (extension != ".h" && extension != ".cpp") continue;

        FileNode n;
        n.path = entry.path();
        n.name = entry.path().filename().string();
        n.isDirectory = false;
        m_ScriptItems.push_back(n);

        if (extension == ".h")
            m_ScriptNames.push_back(entry.path().stem().string());
    }

    std::sort(m_ScriptItems.begin(), m_ScriptItems.end(),
        [](const FileNode& a, const FileNode& b) { return a.name < b.name; });
    std::sort(m_ScriptNames.begin(), m_ScriptNames.end());
}

void FileManagerPanel::drawScriptsTab() {
    if (ImGui::Button("Create Script")) m_ImportScriptPopupOpen = true;
    ImGui::SameLine();
    if (ImGui::Button("Refresh##scripts")) refreshScripts();
    ImGui::Separator();

    if (m_ScriptItems.empty()) {
        ImGui::TextDisabled("No scripts found in the project.");
        ImGui::TextDisabled("Click 'Create Script' to create a new script.");
    }

    float panelWidth = ImGui::GetContentRegionAvail().x;
    int columns = std::max(1, static_cast<int>(panelWidth / (m_ItemSize + 16.0f)));
    ImGui::Columns(columns, "##scriptgrid", false);

    ImVec2 iconSize(m_ItemSize, m_ItemSize);
    std::filesystem::path scrollPath("resources/images/script.svg");

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));

    for (auto& node : m_ScriptItems) {
        ImGui::PushID(node.path.u8string().c_str());

        uint32_t thumb = Thumbnail::loadThumbnailFromSvg(scrollPath, m_ItemSize, m_ItemSize);
        if (thumb != UINT_MAX)
            ImGui::ImageButton("##btn", (ImTextureID)(intptr_t)thumb, iconSize);
        else
            ImGui::Button("[SCR]", iconSize);

        if (ImGui::IsItemClicked()) m_SelectedPath = node.path;

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
                FileTypeController::revealInExplorer(node.path.parent_path());
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
        float indent = (m_ItemSize - textWidth) * 0.5f;
        if (indent > 0) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + indent);
        ImGui::TextUnformatted(display.c_str());
        ImGui::Spacing();

        ImGui::PopID();
        ImGui::NextColumn();
    }

    ImGui::PopStyleColor(3);

    ImGui::Columns(1);
}

void FileManagerPanel::show(bool* open) {
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    if (!ImGui::Begin("File Manager", open, flags)) {
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("##FMTabs")) {
        if (ImGui::BeginTabItem("Assets")) {
            m_ActiveTab = Tab::Assets;
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Scripts")) {
            m_ActiveTab = Tab::Scripts;
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Separator();

    if (m_ImportScriptPopupOpen) {
        ImGui::OpenPopup("Import Script##FM");
        m_ImportScriptPopupOpen = false;
    }
    if (ImGui::BeginPopupModal("Import Script##FM", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Name the script you want to create:");
        ImGui::InputText("##importscript", m_ImportScriptPathBuf, sizeof(m_ImportScriptPathBuf));
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            ScriptingController::createNewScript(m_ImportScriptPathBuf, m_CurrentDir.string());
            refresh();
            memset(m_ImportScriptPathBuf, 0, sizeof(m_ImportScriptPathBuf));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (m_ActiveTab == Tab::Scripts) {
        ImGui::BeginChild("##FMScripts", ImVec2(0, 0), false,
            ImGuiWindowFlags_HorizontalScrollbar);
        drawScriptsTab();
        ImGui::EndChild();

        ImGui::End();
        return;
    }

    if (ImGui::Button("Import")) m_ImportPopupOpen = true;
    ImGui::SameLine();
    if (ImGui::Button("New Folder")) {
        memset(m_NewFolderName, 0, sizeof(m_NewFolderName));
        strcpy(m_NewFolderName, "New Folder");
        m_CreateFolderOpen = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh")) refresh();
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    ImGui::SliderFloat("Size", &m_ItemSize, 48.0f, 128.0f);

    // Never even used this once during testing, so I removed it
    //ImGui::Separator();
    //drawBreadcrumb();
    ImGui::Separator();

    float treeWidth = 200.0f;
    ImGui::BeginChild("##FMTree", ImVec2(treeWidth, 0), true);
    drawFolderTree(m_RootNode);
    ImGui::EndChild();

    if (!m_PendingNavigate.empty()) {
        m_CurrentDir = m_PendingNavigate;
        m_PendingNavigate.clear();
        refresh();
    }

    ImGui::SameLine();

    ImGui::BeginChild("##FMContent", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar);
    drawContentBrowser();
    ImGui::EndChild();

    if (m_CreateFolderOpen) {
        ImGui::OpenPopup("Create Folder##FM");
        m_CreateFolderOpen = false;
    }
    if (ImGui::BeginPopupModal("Create Folder##FM", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Folder name:");
        ImGui::InputText("##foldername", m_NewFolderName, sizeof(m_NewFolderName));
        if (ImGui::Button("Create", ImVec2(120, 0))) {
            createFolder(m_CurrentDir, m_NewFolderName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    if (m_ImportPopupOpen) {
        ImGui::OpenPopup("Import Asset##FM");
        m_ImportPopupOpen = false;
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

// Maybe useful but removed to reduce cluttering
void FileManagerPanel::drawBreadcrumb() {
    std::vector<std::filesystem::path> crumbs;
    std::filesystem::path cur = m_CurrentDir;
    while (cur != m_RootPath.parent_path() && cur != cur.parent_path()) {
        crumbs.push_back(cur);
        if (cur == m_RootPath) break;
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
            m_CurrentDir = crumbs[i];
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

    if (node.path == m_CurrentDir)
        flags |= ImGuiTreeNodeFlags_Selected;

    if (node.path == m_RootPath)
        flags |= ImGuiTreeNodeFlags_DefaultOpen;

    std::string label = "[DIR] " + node.name + "##tree" + node.path.string();
    bool open = ImGui::TreeNodeEx(label.c_str(), flags);

    if (ImGui::IsItemClicked()) {
        m_PendingNavigate = node.path;
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
    int columns = std::max(1, static_cast<int>(panelWidth / (m_ItemSize + 16.0f)));

    ImGui::Columns(columns, "##fmgrid", false);

    for (auto& node : m_CurrentItems) {
        drawItem(node);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
}

void FileManagerPanel::drawItem(FileNode& node) {
    bool selected = (m_SelectedPath == node.path);

    ImGui::PushID(node.path.u8string().c_str());

    if (m_DeletePopupTriggered) {
        if (m_NodeToBeDeleted && m_NodeToBeDeleted == &node) {
            ImGui::OpenPopup(("Delete?##" + m_NodeToBeDeleted->path.string()).c_str());
            m_DeletePopupTriggered = false;
        }
    }

    if (m_NodeToBeDeleted && ImGui::BeginPopupModal(("Delete?##" + m_NodeToBeDeleted->path.string()).c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Delete \"%s\"?\nThis cannot be undone.", m_NodeToBeDeleted->name.c_str());
        ImGui::Separator();

        if (ImGui::Button("Delete", ImVec2(120, 0))) {
            deleteItem(*m_NodeToBeDeleted);
            m_NodeToBeDeleted = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            m_NodeToBeDeleted = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (m_RenamingActive && m_RenamingPath == node.path) {
        ImGui::SetNextItemWidth(m_ItemSize);
        if (ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_AutoSelectAll)) {
            renameItem(node, std::string(m_RenameBuffer));
            m_RenamingActive = false;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            m_RenamingActive = false;
        }
        ImGui::PopID();
        return;
    }

    ImVec2 iconSize(m_ItemSize, m_ItemSize);

    if (selected) {
        ImVec4 selColor(0.26f, 0.59f, 0.98f, 0.35f);
        ImGui::PushStyleColor(ImGuiCol_Button, selColor);
    }

    bool clicked = false;
    if (FileTypeController::isImageFile(node.path)) {
        uint32_t thumb = Thumbnail::loadThumbnailFromImage(node.path);
        if (thumb != UINT_MAX) {
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
        m_SelectedPath = node.path;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (node.isDirectory) {
            m_PendingNavigate = node.path;
        }
        else if (FileTypeController::isSceneFile(node.path)) {
            SceneController::openScene(node.path);
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
    float indent = (m_ItemSize - textWidth) * 0.5f;
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
            memset(m_NewFolderName, 0, sizeof(m_NewFolderName));
            strcpy(m_NewFolderName, "New Folder");
            m_CreateFolderOpen = true;
        }
        if (ImGui::MenuItem("Create Script")) m_ImportScriptPopupOpen = true;
        if (ImGui::MenuItem("Import Asset...")) m_ImportPopupOpen = true;
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
                m_PendingNavigate = node.path;
            }
        }
        else if (FileTypeController::isSceneFile(node.path)) {
            if (ImGui::MenuItem("Open Scene"))
                SceneController::openScene(node.path);
        }
        else if (FileTypeController::isMeshFile(node.path)) {
            std::string assetName;
            if (ImGui::MenuItem("Load Mesh") && AssetController::getAssetName(node.path, assetName)) {
                AssetController::loadMesh(assetName);
            }
        }
        else if (FileTypeController::isImageFile(node.path)) {
            std::string assetName;
            if (ImGui::MenuItem("Load Texture") && AssetController::getAssetName(node.path, assetName)) {
                AssetController::loadTexture(assetName);
            }
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Rename")) {
            m_RenamingActive = true;
            m_RenamingPath = node.path;
            std::string stem = node.isDirectory
                ? node.name
                : node.path.stem().string();
            strncpy(m_RenameBuffer, stem.c_str(), sizeof(m_RenameBuffer) - 1);
        }

        if (ImGui::MenuItem("Show in Explorer")) {
            std::filesystem::path target = node.isDirectory ? node.path : node.path.parent_path();
            FileTypeController::revealInExplorer(target);
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete")) {
            m_DeletePopupTriggered = true;
            m_NodeToBeDeleted = &node;
        }

        ImGui::EndPopup();
    }
}