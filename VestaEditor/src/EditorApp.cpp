#include <EditorApp.h>

#include <GLFW/glfw3.h>

#include <imgui.h>
#include <imguizmo/src/ImGuizmo.h>

#include <core/Application.h>
#include <core/Input.h>
#include <core/Project.h>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/TransformComponent.h>

#include <renderer/Renderer.h>

void EditorApp::onAttach() {
	m_ImGuiLayer.Init(Application::getWindow());

	m_EditorLayer.Init([]() { Application::close(); });
	m_EditorLayer.initFileManager(Project::getRoot());

	// real sizes come from the panels on the first frame
	ImVec2 sceneSize = m_EditorLayer.getSceneViewSize();
	ImVec2 gameSize = m_EditorLayer.getGameViewSize();
	m_SceneFramebuffer.Init(static_cast<unsigned int>(sceneSize.x), static_cast<unsigned int>(sceneSize.y));
	m_GameFramebuffer.Init(static_cast<unsigned int>(gameSize.x), static_cast<unsigned int>(gameSize.y));
}

void EditorApp::onDetach() {
	m_ImGuiLayer.Shutdown();
}

void EditorApp::onUpdate(float dt) {
	handleShortcuts();
	updateCursorCapture();

	m_EditorLayer.editorCamera.update(dt);

	renderSceneView();
	renderGameView();

	m_ImGuiLayer.Begin();
	ImGuizmo::BeginFrame();
	m_EditorLayer.OnUIRender();
	m_ImGuiLayer.End();
}

void EditorApp::handleShortcuts() {
	// Ctrl+Shift+F: move the scene's active camera to the editor camera
	if (Input::isKeyDown(Key::LeftControl) &&
		Input::isKeyDown(Key::LeftShift) &&
		Input::isKeyPressed(Key::F)) {

		Entity camEntity = Application::getCurrentScene()->getActiveCameraEntity();
		if (camEntity.isValid()) {
			TransformComponent camTransform = camEntity.getTransform();
			camTransform.setPosition(m_EditorLayer.editorCamera.position);
			camTransform.setRotation(m_EditorLayer.editorCamera.rotation);
		}
	}
}

// hide the cursor while a widget in the inspector is being drag-edited
void EditorApp::updateCursorCapture() {
	bool dragging = m_EditorLayer.inspectorFocused &&
		ImGui::IsAnyItemActive() &&
		ImGui::IsMouseDragging(ImGuiMouseButton_Left);

	if (dragging == m_CursorCaptured) return;

	m_CursorCaptured = dragging;
	glfwSetInputMode(Application::getWindow(), GLFW_CURSOR,
		dragging ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
}

// scene view, rendered through the editor camera
void EditorApp::renderSceneView() {
	ImVec2 size = m_EditorLayer.getSceneViewSize();
	unsigned int width = static_cast<unsigned int>(size.x);
	unsigned int height = static_cast<unsigned int>(size.y);
	if (width == 0 || height == 0) return;

	float aspect = static_cast<float>(width) / static_cast<float>(height);
	const EditorCamera& camera = m_EditorLayer.editorCamera;

	m_SceneFramebuffer.Resize(width, height);
	m_SceneFramebuffer.Bind();
	Application::getCurrentRenderer()->render(*Application::getCurrentScene(),
		camera.getViewMatrix(), camera.getProjectionMatrix(aspect));
	m_SceneFramebuffer.Unbind();

	m_EditorLayer.sceneViewTexture = m_SceneFramebuffer.GetColorAttachment();
}

// game view, rendered through the scene's active camera, skipped if there is none
void EditorApp::renderGameView() {
	ImVec2 size = m_EditorLayer.getGameViewSize();
	unsigned int width = static_cast<unsigned int>(size.x);
	unsigned int height = static_cast<unsigned int>(size.y);
	if (width == 0 || height == 0) return;

	Scene& scene = *Application::getCurrentScene();
	auto gameCamera = scene.getActiveCameraEntity().getComponent<CameraComponent>();
	if (!gameCamera) return;

	gameCamera->setAspect(static_cast<float>(width) / static_cast<float>(height));

	m_GameFramebuffer.Resize(width, height);
	m_GameFramebuffer.Bind();
	Application::getCurrentRenderer()->render(scene);
	m_GameFramebuffer.Unbind();

	m_EditorLayer.gameViewTexture = m_GameFramebuffer.GetColorAttachment();
}
