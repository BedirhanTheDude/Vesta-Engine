#include <gui/HierarchyPanel.h>

#include <controller/SceneController.h>
#include <controller/EntityController.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <imgui.h>

#include <cstdint>
#include <cstring>
#include <string>

const char* HierarchyPanel::DND_ID = "HIERARCHY_ENTITY";

void HierarchyPanel::drawNode(
    const Entity& entity,
    uint32_t selectedEntityID)
{
    if (!entity.isAlive())
        return;

    const unsigned int entityID = entity.getID();

    TransformComponent transform = entity.getTransform();

    const std::size_t childCount = transform.getChildCount();

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (childCount == 0)
        flags |= ImGuiTreeNodeFlags_Leaf;

    if (selectedEntityID == entityID)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(static_cast<int>(entityID));

    if (renamingEntityID == static_cast<int64_t>(entityID)) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);

        if (ImGui::InputText(
            "##rename",
            renameBuffer,
            sizeof(renameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue |
            ImGuiInputTextFlags_AutoSelectAll))
        {
            if (renameBuffer[0] != '\0')
                entity.setName(renameBuffer);

            renamingEntityID = -1;
        }

        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            renamingEntityID = -1;

        ImGui::PopID();
        return;
    }

    const std::string entityName = entity.getName();

    const bool open = ImGui::TreeNodeEx(
        reinterpret_cast<void*>(
            static_cast<uintptr_t>(entityID)),
        flags,
        "%s",
        entityName.c_str()
    );

    if (ImGui::IsItemClicked())
        EntityController::setSelectedEntity(entity);

    if (ImGui::BeginDragDropSource(
        ImGuiDragDropFlags_SourceAllowNullID))
    {
        ImGui::SetDragDropPayload(
            DND_ID,
            &entityID,
            sizeof(entityID)
        );

        ImGui::Text("Move: %s", entityName.c_str());

        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(DND_ID))
        {
            const unsigned int draggedID =
                *static_cast<const unsigned int*>(payload->Data);

            if (!EntityController::wouldCreateCycle(
                draggedID,
                entityID))
            {
                EntityController::setEntityParent(
                    draggedID,
                    entityID
                );
            }
        }

        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem("##entityCtx")) {

        if (ImGui::MenuItem("Rename")) {
            renamingEntityID = static_cast<int64_t>(entityID);

            std::strncpy(
                renameBuffer,
                entityName.c_str(),
                sizeof(renameBuffer) - 1
            );

            renameBuffer[sizeof(renameBuffer) - 1] = '\0';
        }

        if (transform.getParent().isValid()) {
            if (ImGui::MenuItem("Unparent"))
                transform.setParent(Entity());
        }

        if (ImGui::MenuItem("Copy Entity"))
            EntityController::copyEntity(entityID);

        if (ImGui::MenuItem("Duplicate")) {
            EntityController::copyEntity(entityID);
            EntityController::pasteEntity();
        }

        ImGui::Separator();

        if (ImGui::MenuItem("Delete")) {
            SceneController::getCurrentScene()->removeEntity(entity);

            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();

            if (open)
                ImGui::TreePop();

            ImGui::PopID();
            return;
        }

        ImGui::EndPopup();
    }

    if (open) {

        for (std::size_t i = 0; i < childCount; ++i) {
            Entity child = transform.getChild(i);

            if (child.isAlive())
                drawNode(child, selectedEntityID);
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void HierarchyPanel::show(bool* open)
{
    if (!ImGui::Begin("Hierarchy", open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Add Entity"))
        SceneController::createEntity();

    ImGui::Separator();

    Scene* scene = SceneController::getCurrentScene();

    if (!scene) {
        ImGui::TextDisabled("No scene loaded.");
        ImGui::End();
        return;
    }

    const uint32_t selectedEntityID =
        EntityController::getSelectedEntityID();

    const std::vector<Entity>& entities =
        scene->getEntities();

    for (const Entity& entity : entities) {

        if (!entity.isAlive())
            continue;

        const Entity parent =
            entity.getTransform().getParent();

        if (parent.isValid())
            continue;

        drawNode(entity, selectedEntityID);
    }

    if (ImGui::IsWindowHovered(
        ImGuiHoveredFlags_AllowWhenBlockedByActiveItem) &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Right) &&
        !ImGui::IsAnyItemHovered())
    {
        ImGui::OpenPopup("##HBackgroundCtx");
    }

    if (ImGui::BeginPopup("##HBackgroundCtx")) {

        if (ImGui::MenuItem(
            "Paste Entity",
            nullptr,
            false,
            EntityController::canPasteEntity()))
        {
            EntityController::pasteEntity();
        }

        ImGui::EndPopup();
    }

    if (ImGui::BeginDragDropTarget()) {

        if (const ImGuiPayload* payload =
            ImGui::AcceptDragDropPayload(DND_ID))
        {
            const unsigned int draggedID =
                *static_cast<const unsigned int*>(payload->Data);

            EntityController::unparentEntity(draggedID);
        }

        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}