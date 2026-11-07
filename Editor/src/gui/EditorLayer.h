#pragma once

#include <imgui.h>

#include <functional>

#include <gui/EditorCamera.h>
#include <gui/FileManagerPanel.h>
#include <gui/HierarchyPanel.h>

class Scene;
class Entity;

class EditorLayer {
public:
    void Init(std::function<void()> onExit);

    void initFileManager(const std::filesystem::path& projectRoot);

    void OnUIRender();
    void OnEntityRemoved(Entity* entity);
    ImVec2 getSceneViewSize() const { return m_SceneViewSize; }
    ImVec2 getGameViewSize() const { return m_GameViewSize; }

    EditorCamera editorCamera;
    unsigned int sceneViewTexture = 0;
    unsigned int gameViewTexture = 0;

    std::function<void()> onExit;

    bool inspectorFocused = false;

private:
    void ShowDockspace();
    void ShowMenuBar();
    void ShowStatsPanel();
    void ShowHierarchyPanel();
    void ShowInspectorPanel();
    void ShowConsolePanel();
    void ShowScenePanel();
    void ShowGamePanel();
    ImVec2 m_SceneViewSize = ImVec2(1000, 800);
    ImVec2 m_GameViewSize = ImVec2(1000, 800);

    bool m_ShowStats = true;
    bool m_ShowHierarchy = true;
    bool m_ShowInspector = true;
    bool m_ShowConsole = true;
    bool m_ShowScene = true;
    bool m_ShowGame = true;
    bool m_ShowFileManager = true;

    FileManagerPanel m_FileManager;
    HierarchyPanel m_Hierarchy;
};