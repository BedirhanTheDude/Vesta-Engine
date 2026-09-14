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

    void initFileManager(const std::string& rootAssetsPath, const std::string& scriptsPath);

    void OnUIRender();
    void OnEntityRemoved(Entity* entity);
    ImVec2 getSceneViewSize() const { return sceneViewSize; }
    ImVec2 getGameViewSize() const { return gameViewSize; }

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
    ImVec2 sceneViewSize = ImVec2(1000, 800);
    ImVec2 gameViewSize = ImVec2(1000, 800);

    bool showStats = true;
    bool showHierarchy = true;
    bool showInspector = true;
    bool showConsole = true;
    bool showScene = true;
    bool showGame = true;
    bool showFileManager = true;

    FileManagerPanel fileManager;
    HierarchyPanel hierarchyPanel;
};