#include <gui/HierarchyPanel.h>

#include <controller/SceneController.h>
#include <controller/EntityController.h>

#include <imgui.h>

#include <set>
#include <string>
#include <algorithm>

const char* HierarchyPanel::DND_ID = "HIERARCHY_ENTITY";

void HierarchyPanel::drawNode(unsigned int eID) {
    std::set<unsigned int> childrenIDs = EntityController::getEntityChildIDs(eID);

    ImGuiTreeNodeFlags flags =
        ImGuiTreeNodeFlags_OpenOnArrow |
        ImGuiTreeNodeFlags_OpenOnDoubleClick |
        ImGuiTreeNodeFlags_SpanAvailWidth;

    if (childrenIDs.empty()) flags |= ImGuiTreeNodeFlags_Leaf;
    if (EntityController::getSelectedEntityID() == eID)
        flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID((int)eID);

    if (renamingEntityID == (int64_t)eID) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
        if (ImGui::InputText("##rename", renameBuffer, sizeof(renameBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll)) {
            if (strlen(renameBuffer) > 0)
                EntityController::renameEntity(eID, renameBuffer);
            renamingEntityID = -1;
        }
        if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            renamingEntityID = -1;

        ImGui::PopID();
        return;
    }

    std::string entityName = EntityController::getEntityName(eID);

    bool open = ImGui::TreeNodeEx((void*)(intptr_t)eID,
        flags, "%s", entityName.c_str());

    if (ImGui::IsItemClicked())
        EntityController::setSelectedEntityID(eID);

    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
        ImGui::SetDragDropPayload(DND_ID, &eID, sizeof(unsigned int));
        ImGui::Text("Move: %s", entityName.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DND_ID)) {
            unsigned int draggedID = *(unsigned int*)payload->Data;

            if (!EntityController::wouldCreateCycle(draggedID, eID))
                EntityController::setEntityParent(draggedID, eID);
        }
        ImGui::EndDragDropTarget();
    }

    if (ImGui::BeginPopupContextItem("##entityCtx")) {
        if (ImGui::MenuItem("Rename")) {
            renamingEntityID = eID;
            strncpy(renameBuffer, entityName.c_str(), sizeof(renameBuffer) - 1);
            renameBuffer[sizeof(renameBuffer) - 1] = '\0';
        }
        if (EntityController::entityHasParent(eID)) {
            if (ImGui::MenuItem("Unparent"))
                EntityController::unparentEntity(eID);
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Delete")) {
            EntityController::removeEntity(eID);
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
            if (open) ImGui::TreePop();
            ImGui::PopID();
            return;
        }
        ImGui::EndPopup();
    }

    if (open) {
        for (unsigned int childID : childrenIDs)
            drawNode(childID);
        ImGui::TreePop();
    }
    ImGui::PopID();
}

void HierarchyPanel::show(bool* open) {
    if (!ImGui::Begin("Hierarchy", open)) {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Add Entity"))
        SceneController::createEntity();

    ImGui::Separator();

    const auto& entityIDs = EntityController::getAllEntityIDs();

    for (unsigned int eID : entityIDs) {
        if (EntityController::entityHasParent(eID)) continue;
        drawNode(eID);
    }

    ImGui::InvisibleButton("##hierarchyBg",
        ImVec2(ImGui::GetContentRegionAvail().x,
            std::max(ImGui::GetContentRegionAvail().y, 20.0f)));

    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(DND_ID)) {
            unsigned int dragged = *(unsigned int*)payload->Data;
            EntityController::unparentEntity(dragged);
        }
        ImGui::EndDragDropTarget();
    }

    ImGui::End();
}