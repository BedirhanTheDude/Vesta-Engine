#include <controller/EditorSelectionController.h>

#include <controller/EntityController.h>

#include <core/Application.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/MeshComponent.h>
#include <renderer/Mesh.h>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

void EditorSelectionController::selectEntityFromViewport(
    float mouseX,
    float mouseY,
    float viewportWidth,
    float viewportHeight,
    const glm::mat4& view,
    const glm::mat4& projection
) {
    if (viewportWidth <= 0.0f || viewportHeight <= 0.0f) return;

    Scene* scene = Application::getCurrentScene();
    if (!scene) return;

    ColourPickingRenderer& picker = getInstance().getColourPicker();
    picker.resize((int)viewportWidth, (int)viewportHeight);

    picker.renderPickingPass(scene, view, projection,
        [&picker](unsigned int shader, Entity* entity, const glm::mat4& mvp) {
            glUseProgram(shader);
            glUniformMatrix4fv(glGetUniformLocation(shader, "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
            glm::vec3 colour = picker.getPickedColour(entity);
            glUniform3fv(glGetUniformLocation(shader, "uColour"), 1, glm::value_ptr(colour));

            MeshComponent* mc = entity->getComponent<MeshComponent>();
            if (mc && mc->getMesh()) mc->getMesh()->draw();
        });

    Entity* clicked = picker.pickEntity(mouseX, mouseY);
    EntityController::setSelectedEntity(clicked);
}
