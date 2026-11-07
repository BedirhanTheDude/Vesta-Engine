#pragma once

#include <glm/glm.hpp>
#include <renderer/Colourpickingrenderer.h>

class Entity;
class Scene;

namespace EditorSelectionController {

    inline ColourPickingRenderer& getColourPicker() { 
        static ColourPickingRenderer instance;
        return instance;
    }

    void selectEntityFromViewport(
        float mouseX,
        float mouseY,
        float viewportWidth,
        float viewportHeight,
        const glm::mat4& view,
        const glm::mat4& projection
    );
}