#include <gui/ImGuiLayer.h>

#define GL_SILENCE_DEPRECATION
#define GLFW_INCLUDE_NONE

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <controller/ProjectController.h>
#include <controller/WindowController.h>

static void setupImGuiStyle()
{
	// Red Font style by aiekick from ImThemes
	// Slightly modified to taste
	ImGuiStyle& style = ImGui::GetStyle();

	style.Alpha = 1.0f;
	style.DisabledAlpha = 0.6f;
	style.WindowPadding = ImVec2(8.0f, 8.0f);
	style.WindowRounding = 0.0f;
	style.WindowBorderSize = 1.0f;
	style.WindowMinSize = ImVec2(32.0f, 32.0f);
	style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
	style.WindowMenuButtonPosition = ImGuiDir_Left;
	style.ChildRounding = 0.0f;
	style.ChildBorderSize = 1.0f;
	style.PopupRounding = 0.0f;
	style.PopupBorderSize = 1.0f;
	style.FramePadding = ImVec2(4.0f, 3.0f);
	style.FrameRounding = 1.0f;
	style.FrameBorderSize = 0.0f;
	style.ItemSpacing = ImVec2(8.0f, 4.0f);
	style.ItemInnerSpacing = ImVec2(4.0f, 4.0f);
	style.CellPadding = ImVec2(4.0f, 2.0f);
	style.IndentSpacing = 21.0f;
	style.ColumnsMinSpacing = 6.0f;
	style.ScrollbarSize = 14.0f;
	style.ScrollbarRounding = 9.0f;
	style.GrabMinSize = 10.0f;
	style.GrabRounding = 0.0f;
	style.TabRounding = 4.0f;
	style.TabBorderSize = 0.0f;
	style.ColorButtonPosition = ImGuiDir_Right;
	style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
	style.SelectableTextAlign = ImVec2(0.0f, 0.0f);

	style.Colors[ImGuiCol_Text] = ImVec4(0.7490196f, 0.7490196f, 0.7490196f, 1.0f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.34901962f, 0.34901962f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(0.04f, 0.04f, 0.04f, 0.94f);
	style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(0.078431375f, 0.078431375f, 0.078431375f, 0.94f);
	style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.5f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.54f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.36862746f, 0.13725491f, 0.13725491f, 0.67f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.3882353f, 0.2f, 0.2f, 0.67f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(0.066f, 0.066f, 0.066f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.47843137f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.47843137f, 0.15686275f, 0.15686275f, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.13725491f, 0.13725491f, 0.13725491f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.019607844f, 0.019607844f, 0.019607844f, 0.53f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.30980393f, 0.30980393f, 0.30980393f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40784314f, 0.40784314f, 0.40784314f, 1.0f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.50980395f, 0.50980395f, 0.50980395f, 1.0f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(0.5568628f, 0.09803922f, 0.09803922f, 1.0f);
	style.Colors[ImGuiCol_CheckboxSelectedBg] = ImVec4(0.327f, 0.124f, 0.098f, 0.735f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.1882353f, 0.1882353f, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.8862745f, 0.0f, 0.1882353f, 1.0f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.1882353f, 0.1882353f, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.8f, 0.16862746f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.8862745f, 0.0f, 0.1882353f, 1.0f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.32941177f, 0.34901962f, 0.35686275f, 0.53f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75686276f, 0.2784314f, 0.2784314f, 0.67f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.46666667f, 0.46666667f, 0.46666667f, 0.67f);
	style.Colors[ImGuiCol_Separator] = ImVec4(0.479f, 0.075f, 0.075f, 1.0f);
	style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.31764707f, 0.31764707f, 0.31764707f, 1.0f);
	style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.31764707f, 0.31764707f, 0.31764707f, 1.0f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(1.0f, 1.0f, 1.0f, 0.85f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.6f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.9f);
	style.Colors[ImGuiCol_Tab] = ImVec4(0.06666667f, 0.06666667f, 0.06666667f, 0.51f);
	style.Colors[ImGuiCol_TabHovered] = ImVec4(0.85882354f, 0.22745098f, 0.22745098f, 0.67f);
	style.Colors[ImGuiCol_TabActive] = ImVec4(0.1882353f, 0.1882353f, 0.1882353f, 0.57f);
	style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.047058824f, 0.047058824f, 0.047058824f, 0.9f);
	style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.12941177f, 0.12941177f, 0.12941177f, 0.74f);
	style.Colors[ImGuiCol_TabSelectedOverline] = ImVec4(0.957f, 0.272f, 0.272f, 1.0f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.60784316f, 0.60784316f, 0.60784316f, 1.0f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.42745098f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.8980392f, 0.69803923f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.6f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(0.1882353f, 0.1882353f, 0.2f, 1.0f);
	style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(0.30980393f, 0.30980393f, 0.34901962f, 1.0f);
	style.Colors[ImGuiCol_TableBorderLight] = ImVec4(0.22745098f, 0.22745098f, 0.24705882f, 1.0f);
	style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
	style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.07f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 0.35f);
	style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.0f, 1.0f, 0.0f, 0.9f);
	style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.25882354f, 0.5882353f, 0.9764706f, 1.0f);
	style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
	style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
	style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.35f);
}

void ImGuiLayer::Init(GLFWwindow* window) {
    m_Window = window;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	ImFont* font = io.Fonts->AddFontFromFileTTF("resources/fonts/IBMPlexMono-Regular.ttf", 16.0f);
	io.FontDefault = font;

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;

    ImGui_ImplGlfw_InitForOpenGL(window, true);

#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");
#else
    ImGui_ImplOpenGL3_Init("#version 330");
#endif

	setupImGuiStyle();

    if (ProjectController::isProjectOpen()) {
        std::string projName;
        if (ProjectController::tryGetProjectName(projName)) {
            WindowController::addWindowSuffix(projName);
        }
        else {
            WindowController::addWindowSuffix("<blank>");
        }
    }
    else {
        WindowController::addWindowSuffix("[no-project]");
    }
}

void ImGuiLayer::Begin() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiLayer::End() {
    ImGui::Render();

    int displayWidth = 0;
    int displayHeight = 0;
    glfwGetFramebufferSize(m_Window, &displayWidth, &displayHeight);
    glViewport(0, 0, displayWidth, displayHeight);

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ImGuiLayer::Shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}