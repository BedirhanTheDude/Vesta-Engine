#pragma once

#include <glm/glm.hpp>
#include <renderer/Colourpickingrenderer.h>

class Entity;
class Scene;

class EditorSelectionController {
public:
	static EditorSelectionController& getInstance() {
		static EditorSelectionController instance;
		return instance;
	}
	ColourPickingRenderer& getColourPicker() { return picker; }

	// renders a fresh picking pass for the current scene, then reads back the entity
	// under (mouseX, mouseY) and makes it the selection. Keeps all GL out of the view.
	static void selectEntityFromViewport(
		float mouseX,
		float mouseY,
		float viewportWidth,
		float viewportHeight,
		const glm::mat4& view,
		const glm::mat4& projection
	);

private:
	EditorSelectionController() = default;
	ColourPickingRenderer picker;
};
