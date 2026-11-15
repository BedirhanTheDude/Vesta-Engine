#pragma once

#include <core/Layer.h>
#include <renderer/ViewportFramebuffer.h>

#include <gui/ImGuiLayer.h>
#include <gui/EditorLayer.h>

// renders the scene and game viewports offscreen, draws the ImGui editor on top
// supposed to be exclusive with the final game runtime so the Engine Application runs with only one layer at a time
class EditorApp : public Layer {
public:
	void onAttach() override;
	void onDetach() override;
	void onUpdate(float dt) override;

private:
	void handleShortcuts();
	void updateCursorCapture();
	void renderSceneView();
	void renderGameView();

	ImGuiLayer m_ImGuiLayer;
	EditorLayer m_EditorLayer;

	ViewportFramebuffer m_SceneFramebuffer;
	ViewportFramebuffer m_GameFramebuffer;

	bool m_CursorCaptured = false;
};
