#include "EditorLayer.h"

#include "imgui.h"

#include "ImGuizmo.h"

#include <core/Input.h>

#include <controller/SceneController.h>
#include <controller/EntityController.h>
#include <controller/AssetController.h>
#include <controller/ComponentController.h>
#include <controller/TransformController.h>
#include <controller/EditorSelectionController.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include <cstdint>
#include <map>

void EditorLayer::Init(std::function<void()> onExit) {
    this->onExit = onExit;
}

void EditorLayer::initFileManager(const std::string& rootAssetsPath, const std::string& scriptsPath) {
    fileManager.init(rootAssetsPath);
    fileManager.initScriptsPath(scriptsPath);
}

void EditorLayer::OnUIRender() {
    ShowDockspace();
    ShowMenuBar();

    if (showStats)
        ShowStatsPanel();

    if (showHierarchy)
        ShowHierarchyPanel();

    if (showInspector)
        ShowInspectorPanel();

    if (showConsole)
        ShowConsolePanel();

    if (showScene)
        ShowScenePanel();

    if (showGame)
        ShowGamePanel();

    if (showFileManager)
        fileManager.show(&showFileManager);
}

void EditorLayer::ShowDockspace() {
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    windowFlags |= ImGuiWindowFlags_NoTitleBar;
    windowFlags |= ImGuiWindowFlags_NoCollapse;
    windowFlags |= ImGuiWindowFlags_NoResize;
    windowFlags |= ImGuiWindowFlags_NoMove;
    windowFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus;
    windowFlags |= ImGuiWindowFlags_NoNavFocus;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockSpace", nullptr, windowFlags);
    ImGui::PopStyleVar(2);

    ImGuiID dockspaceID = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_PassthruCentralNode);

    ImGui::End();
}

void EditorLayer::ShowMenuBar() {
    static bool openNewScene = false;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Scene"))
                openNewScene = true;

            if (ImGui::MenuItem("Open Scene", "Ctrl+O")) {
                SceneController::loadScene(); // NOTE: add a dialog to select scene file
            }
            if (ImGui::MenuItem("Save Scene", "Ctrl+S")) {
                SceneController::saveScene();
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) {
                if (onExit) onExit();
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window")) {
            ImGui::MenuItem("Stats", nullptr, &showStats);
            ImGui::MenuItem("Hierarchy", nullptr, &showHierarchy);
            ImGui::MenuItem("Inspector", nullptr, &showInspector);
            ImGui::MenuItem("Console", nullptr, &showConsole);
            ImGui::MenuItem("Scene", nullptr, &showScene);
            ImGui::MenuItem("Game", nullptr, &showGame);
            ImGui::EndMenu();
        }

        ImGui::Separator();

        if (SceneController::sceneExists()) {
            if (SceneController::sceneIsPlaying()) {
                if (ImGui::MenuItem("Stop", "F5")) {
                    SceneController::stopScene();
                }
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 1.0f, 0.2f, 1.0f));
                ImGui::Text("PLAYING");
                ImGui::PopStyleColor();
            }
            else {
                if (ImGui::MenuItem("Play", "F5")) {
                    SceneController::playScene();
                }
            }
        }

        ImGui::EndMainMenuBar();
    }

    static bool focusName = false;

    if (openNewScene) {
        ImGui::OpenPopup("##new_scene");
        openNewScene = false;
        focusName = true;
    }

    if (ImGui::BeginPopupModal("##new_scene", nullptr,
        ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar)) {

        static char sceneName[128] = "";
        ImGui::Text("Scene name");
        ImGui::Separator();
        ImGui::SetNextItemWidth(260.0f);
        if (focusName) {
            ImGui::SetKeyboardFocusHere();
            focusName = false;
        }
        bool confirm = ImGui::InputText("##name", sceneName, sizeof(sceneName),
            ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::Spacing();
        if (ImGui::Button("Create", ImVec2(126, 0)) || confirm) {
            if (strlen(sceneName) > 0) {
                SceneController::newScene(sceneName);
                sceneName[0] = '\0';
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(126, 0))) {
            sceneName[0] = '\0';
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void EditorLayer::ShowStatsPanel() {
    ImGui::Begin("Stats", &showStats);

    ImGuiIO& io = ImGui::GetIO();
    ImGui::Text("FPS: %.1f", io.Framerate);

    if (io.Framerate > 0.0f)
        ImGui::Text("Frame Time: %.3f ms", 1000.0f / io.Framerate);
    else
        ImGui::Text("Frame Time: N/A");

    ImGui::Separator();
    ImGui::Text("Renderer: OpenGL");
    ImGui::Text("Editor Mode: Active");

    ImGui::End();
}

void EditorLayer::ShowHierarchyPanel() {
    hierarchyPanel.show(&showHierarchy);
}

void EditorLayer::ShowInspectorPanel() {
    // collapsing header with a right-aligned "X" remove button
    static auto componentHeader = [](const char* label, bool* removeFlag,
        ImGuiTreeNodeFlags extraFlags = 0) -> bool {
            bool open = ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowOverlap | extraFlags);

            if (removeFlag) {
                ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - 20.0f);

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 0.6f));

                std::string btnId = std::string("X##rm") + label;
                if (ImGui::SmallButton(btnId.c_str())) *removeFlag = true;
                ImGui::PopStyleColor(2);
            }

            return open;
        };

    // editable property: writes straight through prop.ptr, then fires the optional callback
    static auto propertyField = [](const Property& prop) -> bool {
        bool changed = false;
        const std::string& propName = prop.name;

        ImGui::PushID(prop.ptr);
        switch (prop.type) {
        case PropertyType::Float:
            changed = ImGui::DragFloat(propName.c_str(), static_cast<float*>(prop.ptr), 0.1f);
            break;
        case PropertyType::Double: {
            float temp = static_cast<float>(*static_cast<double*>(prop.ptr));
            if (ImGui::DragFloat(propName.c_str(), &temp, 0.1f)) {
                *static_cast<double*>(prop.ptr) = static_cast<double>(temp);
                changed = true;
            }
            break;
        }
        case PropertyType::Int:
            changed = ImGui::DragInt(propName.c_str(), static_cast<int*>(prop.ptr));
            break;
        case PropertyType::Bool:
            changed = ImGui::Checkbox(propName.c_str(), static_cast<bool*>(prop.ptr));
            break;
        case PropertyType::Color:
            changed = ImGui::ColorEdit3(propName.c_str(), &static_cast<glm::vec3*>(prop.ptr)->x);
            break;
        case PropertyType::Vec2:
            changed = ImGui::DragFloat2(propName.c_str(), &static_cast<glm::vec2*>(prop.ptr)->x, 0.1f);
            break;
        case PropertyType::Vec3:
            changed = ImGui::DragFloat3(propName.c_str(), &static_cast<glm::vec3*>(prop.ptr)->x, 0.1f);
            break;
        case PropertyType::Vec4:
            changed = ImGui::DragFloat4(propName.c_str(), &static_cast<glm::vec4*>(prop.ptr)->x, 0.1f);
            break;
        case PropertyType::Mat3: {
            auto* m = static_cast<glm::mat3*>(prop.ptr);
            ImGui::Text("%s", propName.c_str());
            std::string id = "##" + propName;
            changed |= ImGui::DragFloat3((id + "0").c_str(), &(*m)[0][0], 0.1f);
            changed |= ImGui::DragFloat3((id + "1").c_str(), &(*m)[1][0], 0.1f);
            changed |= ImGui::DragFloat3((id + "2").c_str(), &(*m)[2][0], 0.1f);
            break;
        }
        case PropertyType::Mat4: {
            auto* m = static_cast<glm::mat4*>(prop.ptr);
            ImGui::Text("%s", propName.c_str());
            std::string id = "##" + propName;
            changed |= ImGui::DragFloat4((id + "0").c_str(), &(*m)[0][0], 0.1f);
            changed |= ImGui::DragFloat4((id + "1").c_str(), &(*m)[1][0], 0.1f);
            changed |= ImGui::DragFloat4((id + "2").c_str(), &(*m)[2][0], 0.1f);
            changed |= ImGui::DragFloat4((id + "3").c_str(), &(*m)[3][0], 0.1f);
            break;
        }
        case PropertyType::Enum: {
            if (!prop.enumValues.empty()) {
                int current = *static_cast<int*>(prop.ptr);
                if (ImGui::Combo(propName.c_str(), &current,
                    prop.enumValues.data(), static_cast<int>(prop.enumValues.size()))) {
                    *static_cast<int*>(prop.ptr) = current;
                    changed = true;
                }
            }
            break;
        }
        default:
            break;
        }
        if (changed && prop.callback) prop.callback();
        ImGui::PopID();
        return changed;
        };

    static auto callbackField = [](const CallbackProperty& prop) -> bool {
        if (!prop.callback) return false; // no sink, nothing to apply
        bool changed = false;
        const std::string& propName = prop.name;

        ImGui::PushID(prop.ptr);
        switch (prop.type) {
        case CallbackPropertyType::Float: {
            float temp = *static_cast<const float*>(prop.ptr);
            if (ImGui::DragFloat(propName.c_str(), &temp, 0.1f)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Int: {
            int temp = *static_cast<const int*>(prop.ptr);
            if (ImGui::DragInt(propName.c_str(), &temp)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Bool: {
            bool temp = *static_cast<const bool*>(prop.ptr);
            if (ImGui::Checkbox(propName.c_str(), &temp)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Color: {
            glm::vec3 temp = *static_cast<const glm::vec3*>(prop.ptr);
            if (ImGui::ColorEdit3(propName.c_str(), &temp.x)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Vec2: {
            glm::vec2 temp = *static_cast<const glm::vec2*>(prop.ptr);
            if (ImGui::DragFloat2(propName.c_str(), &temp.x, 0.1f)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Vec3: {
            glm::vec3 temp = *static_cast<const glm::vec3*>(prop.ptr);
            if (ImGui::DragFloat3(propName.c_str(), &temp.x, 0.1f)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Vec4: {
            glm::vec4 temp = *static_cast<const glm::vec4*>(prop.ptr);
            if (ImGui::DragFloat4(propName.c_str(), &temp.x, 0.1f)) { prop.callback(&temp); changed = true; }
            break;
        }
        case CallbackPropertyType::Enum: {
            if (!prop.enumValues.empty()) {
                int temp = *static_cast<const int*>(prop.ptr);
                if (ImGui::Combo(propName.c_str(), &temp,
                    prop.enumValues.data(), static_cast<int>(prop.enumValues.size()))) {
                    prop.callback(&temp); changed = true;
                }
            }
            break;
        }
        default:
            break;
        }
        ImGui::PopID();
        return changed;
        };

    // drag-drop resource slot
    static auto payloadField = [](unsigned int entityID, unsigned int groupIdx, unsigned int payloadIdx, const Payload& payload) {
        std::string valueName = AssetController::getResourceName(payload);
        if (valueName.empty()) valueName = "None";

        ImGui::Text("%s: %s", payload.filename.c_str(), valueName.c_str());
        ImGui::SameLine();
        ImGui::TextDisabled(" (drop file here)");

        if (ImGui::BeginDragDropTarget()) {
            if (const ImGuiPayload* pl =
                ImGui::AcceptDragDropPayload(FileManagerPanel::payload)) {
                std::string path(static_cast<const char*>(pl->Data), pl->DataSize - 1);
                std::string filename = std::filesystem::path(path).filename().string();
                switch (payload.type) {
                case PayloadType::Mesh:
                    AssetController::setModel(entityID, filename);
                    break;
                case PayloadType::Texture:
                    AssetController::setTexture(entityID, groupIdx, payloadIdx, filename);
                    break;
                }
            }
            ImGui::EndDragDropTarget();
        }
        };

    ImGui::Begin("Inspector", &showInspector);

    inspectorFocused = ImGui::IsWindowFocused();

    int64_t selectedEntityID = EntityController::getSelectedEntityID();

    if (selectedEntityID == -1) {
        ImGui::Text("No entity selected.");
        ImGui::End();
        return;
    }

    unsigned int entityID = static_cast<unsigned int>(selectedEntityID);

    std::string entityName = EntityController::getEntityName(entityID);

    ImGui::Text("Selected Entity: %s", entityName.c_str());
    ImGui::SameLine();
    ImGui::TextDisabled("(drop script here)");
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* pl =
            ImGui::AcceptDragDropPayload(FileManagerPanel::scriptPayload)) {
            std::string path(static_cast<const char*>(pl->Data), pl->DataSize - 1);
            std::string scriptName = std::filesystem::path(path).stem().string();
            ComponentController::createAndBindComponent(scriptName, entityID);
        }
        ImGui::EndDragDropTarget();
    }
    ImGui::Separator();

    // Transform is special-cased: it lives on Entity directly, not in the component map.
    if (componentHeader("Transform", nullptr)) {
        glm::vec3 position = TransformController::getPosition(entityID);
        glm::vec3 rotation = TransformController::getRotation(entityID);
        glm::vec3 scale = TransformController::getScale(entityID);

        if (ImGui::DragFloat3("Position", &position.x, 0.1f))
            TransformController::setPosition(entityID, position);

        if (ImGui::DragFloat3("Rotation", &rotation.x, 0.1f))
            TransformController::setRotation(entityID, rotation);

        if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.1f, 100.0f))
            TransformController::setScale(entityID, scale);
    }

    const std::map<unsigned int, unsigned int>& componentIdxMap =
        EntityController::getComponentIdxMap(entityID);

    unsigned int componentToRemove = 0;
    bool hasComponentToRemove = false;

    for (const auto& [UID, idx] : componentIdxMap) {
        ImGui::PushID(static_cast<int>(UID));

        std::string compName = ComponentController::componentUIDToString(UID);
        bool removeComp = false;

        if (componentHeader(compName.c_str(), &removeComp, ImGuiTreeNodeFlags_DefaultOpen)) {
            for (const PropertyGroup& group :
                ComponentController::getComponentPropertyGroups(entityID, UID)) {
                if (!group.name.empty()) ImGui::TextUnformatted(group.name.c_str());
                for (const auto& [pidx, prop] : group.properties)
                    propertyField(prop);
            }

            for (const CallbackPropertyGroup& group :
                ComponentController::getComponentCallbackPropertyGroups(entityID, UID)) {
                if (!group.name.empty()) ImGui::TextUnformatted(group.name.c_str());
                for (const auto& [pidx, prop] : group.properties)
                    callbackField(prop);
            }   

            const auto& groups = ComponentController::getComponentPayloadGroups(entityID, UID);
            for (unsigned int groupIdx = 0; groupIdx < groups.size(); groupIdx++) {
                const auto& group = groups.at(groupIdx);
                if (!group.name.empty()) ImGui::TextUnformatted(group.name.c_str());
                for (const auto& [pidx, payload] : group.payloads)
                    payloadField(entityID, groupIdx, pidx, payload);
            }
        }

        if (removeComp) { componentToRemove = UID; hasComponentToRemove = true; }
        ImGui::PopID();
    }

    // defer component removal
    if (hasComponentToRemove)
        ComponentController::removeComponent(entityID, componentToRemove);

    ImGui::Separator();
    if (ImGui::Button("Add Component"))
        ImGui::OpenPopup("AddComponentPopup");

    if (ImGui::BeginPopup("AddComponentPopup")) {
        for (const std::string& name : ComponentController::getAvailableComponentNames()) {
            if (ComponentController::entityHasComponent(entityID, name)) continue;
            if (ImGui::MenuItem(name.c_str()))
                ComponentController::createAndBindComponent(name, entityID);
        }

        ImGui::Separator();

        auto scriptNames = fileManager.getScriptNames();
        if (scriptNames.empty()) {
            ImGui::TextDisabled("No scripts found");
            ImGui::TextDisabled("(add scripts via File Manager > Scripts tab)");
        }
        else if (ImGui::BeginMenu("Script")) {
            for (auto& scriptName : scriptNames) {
                size_t dot = scriptName.find_last_of('.');
                if (dot == std::string::npos) continue;
                if (scriptName.substr(dot) != ".h") continue;

                std::string base = scriptName.substr(0, dot);
                if (ImGui::MenuItem(base.c_str()))
                    ComponentController::createAndBindComponent(base, entityID);
            }
            ImGui::EndMenu();
        }

        ImGui::EndPopup();
    }

    ImGui::End();
}

void EditorLayer::ShowConsolePanel() {
    ImGui::Begin("Console", &showConsole);

    ImGui::TextWrapped("[Info] Engine initialized successfully.");
    ImGui::TextWrapped("[Info] ImGui editor loaded.");

    if (SceneController::sceneExists())
        ImGui::TextWrapped("[Debug] Scene connected.");
    else
        ImGui::TextWrapped("[Debug] No scene connected.");

    ImGui::End();
}

void EditorLayer::ShowScenePanel() {
    static ImGuizmo::OPERATION op = ImGuizmo::TRANSLATE;
    static bool gizmoLocal = true;

    ImGui::Begin("Scene", &showScene);

    if (ImGui::Button("Translate")) op = ImGuizmo::TRANSLATE;
    ImGui::SameLine();
    if (ImGui::Button("Rotate")) op = ImGuizmo::ROTATE;
    ImGui::SameLine();
    if (ImGui::Button("Scale")) op = ImGuizmo::SCALE;
    ImGui::SameLine();
    if (ImGui::Button(gizmoLocal ? "Local" : "World")) gizmoLocal = !gizmoLocal;

    if (ImGui::IsWindowFocused() && !Input::isMouseDown(MouseButton::Right)) {
        if (ImGui::IsKeyPressed(ImGuiKey_W)) op = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E)) op = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R)) op = ImGuizmo::SCALE;
    }

    ImVec2 imagePos = ImGui::GetCursorScreenPos();
    ImVec2 imageSize = ImGui::GetContentRegionAvail();
    sceneViewSize = imageSize;

    if (sceneViewTexture && imageSize.x > 0 && imageSize.y > 0) {
        ImGui::Image((ImTextureID)(intptr_t)sceneViewTexture, imageSize,
            ImVec2(0, 1), ImVec2(1, 0));

        float aspect = imageSize.x / imageSize.y;
        glm::mat4 view = editorCamera.getViewMatrix();
        glm::mat4 proj = editorCamera.getProjectionMatrix(aspect);

        // click-to-select: the controller renders the picking pass and updates the selection
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left) && !ImGuizmo::IsOver()) {
            ImVec2 mouse = ImGui::GetMousePos();
            EditorSelectionController::selectEntityFromViewport(
                mouse.x - imagePos.x, mouse.y - imagePos.y,
                imageSize.x, imageSize.y, view, proj);
        }

        int64_t selectedID = EntityController::getSelectedEntityID();
        if (selectedID != -1) {
            unsigned int entityID = static_cast<unsigned int>(selectedID);
            glm::mat4 model = TransformController::getWorldMatrix(entityID);

            ImGuizmo::SetOrthographic(false);
            ImGuizmo::SetDrawlist();
            ImGuizmo::SetRect(imagePos.x, imagePos.y, imageSize.x, imageSize.y);

            if (ImGuizmo::Manipulate(
                glm::value_ptr(view), glm::value_ptr(proj),
                op, gizmoLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD,
                glm::value_ptr(model))) {

                // gizmo produces a world matrix; convert to parent-local before decomposing
                glm::mat4 parentWorld = TransformController::getParentWorldMatrix(entityID);
                glm::mat4 localMatrix = glm::inverse(parentWorld) * model;

                glm::vec3 pos, rot, scl;
                ImGuizmo::DecomposeMatrixToComponents(
                    glm::value_ptr(localMatrix),
                    glm::value_ptr(pos), glm::value_ptr(rot), glm::value_ptr(scl));

                TransformController::setPosition(entityID, pos);
                TransformController::setRotation(entityID, rot);
                TransformController::setScale(entityID, scl);
            }
        }
    }

    ImGui::End();
}

void EditorLayer::ShowGamePanel() {
    ImGui::Begin("Game", &showGame);
    gameViewSize = ImGui::GetContentRegionAvail();
    if (gameViewTexture && gameViewSize.x > 0 && gameViewSize.y > 0) {
        ImGui::Image((ImTextureID)(intptr_t)gameViewTexture, gameViewSize,
            ImVec2(0, 1), ImVec2(1, 0));
    }
    ImGui::End();
}

void EditorLayer::OnEntityRemoved(Entity* entity) {
    EntityController::setSelectedEntity(nullptr);
}
