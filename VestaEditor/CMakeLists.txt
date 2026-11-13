# VestaEditor.lib

set(EDITOR_APP_SRC
    src/EditorApp.cpp
)

set(GUI_SRC
    src/gui/EditorCamera.cpp
    src/gui/EditorLayer.cpp
    src/gui/FileManagerPanel.cpp
    src/gui/HierarchyPanel.cpp
    src/gui/ImGuiLayer.cpp
    src/gui/Thumbnail.cpp
    src/gui/SvgPixels.cpp
)

set(CONTROLLER_SRC
    src/controller/AssetController.cpp
    src/controller/ComponentController.cpp
    src/controller/EditorSelectionController.cpp
    src/controller/EntityController.cpp
    src/controller/FileTypeController.cpp
    src/controller/ProjectController.cpp
    src/controller/SceneController.cpp
    src/controller/ScriptingController.cpp
    src/controller/TransformController.cpp
    src/controller/WindowController.cpp
)

set(PLATFORM_SRC
    src/platform/FileDialog.cpp
)

set(IMGUI_SRC
    ${VESTA_EXTERNAL_DIR}/imgui/imgui.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/imgui_draw.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/imgui_tables.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/imgui_widgets.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/imgui_demo.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${VESTA_EXTERNAL_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${VESTA_EXTERNAL_DIR}/imguizmo/src/ImGuizmo.cpp
)

add_library(VestaEditor STATIC
    ${EDITOR_APP_SRC}
    ${GUI_SRC}
    ${CONTROLLER_SRC}
    ${PLATFORM_SRC}
    ${IMGUI_SRC}
)

target_include_directories(VestaEditor PUBLIC
    src
    ${VESTA_EXTERNAL_DIR}/imgui
    ${VESTA_EXTERNAL_DIR}/imgui/backends
    ${VESTA_EXTERNAL_DIR}/imguizmo
    ${VESTA_EXTERNAL_DIR}/lunasvg
)

target_link_libraries(VestaEditor PUBLIC Vesta)
target_link_libraries(VestaEditor PRIVATE lunasvg)

# editor resources (fonts, icons) are loaded relative to the working directory
file(COPY resources DESTINATION ${VESTA_OUTPUT_DIR})
