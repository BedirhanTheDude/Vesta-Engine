cmake_minimum_required(VERSION 3.16)
project(GameEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PHYSX_DIR ${CMAKE_SOURCE_DIR}/external/physx)
set(PHYSX_LIB_DIR ${PHYSX_DIR}/$<IF:$<CONFIG:Debug>,debug,release>)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(external/glfw)
set(BUILD_SHARED_LIBS OFF)

find_package(OpenGL REQUIRED)

# Engine sources
# Engine sources
set(PHYSICS_SRC
    Engine/src/physics/CollisionShape.cpp
    Engine/src/physics/PhysicsMaterial.cpp
    Engine/src/physics/PhysicsWorld.cpp
)

set(PHYSICS_BODY_SRC
    Engine/src/physics/body/DynamicPhysicsBody.cpp
    Engine/src/physics/body/PhysicsBody.cpp
    Engine/src/physics/body/StaticPhysicsBody.cpp
)

set(SCENE_SRC
    Engine/src/scene/Entity.cpp
    Engine/src/scene/Scene.cpp
)

set(SCENE_COMPONENT_SRC
    Engine/src/scene/components/BehaviourComponent.cpp
    Engine/src/scene/components/CameraComponent.cpp
    Engine/src/scene/components/ComponentFactory.cpp
    Engine/src/scene/components/DirectionalLightComponent.cpp
    Engine/src/scene/components/MaterialComponent.cpp
    Engine/src/scene/components/MeshComponent.cpp
    Engine/src/scene/components/PointLightComponent.cpp
    Engine/src/scene/components/RigidBodyComponent.cpp
    Engine/src/scene/components/TransformComponent.cpp
)

set(SCENE_SYSTEM_SRC
    Engine/src/scene/system/CameraSystem.cpp
    Engine/src/scene/system/DirectionalLightSystem.cpp
    Engine/src/scene/system/MaterialSystem.cpp
    Engine/src/scene/system/MeshSystem.cpp
    Engine/src/scene/system/PointLightSystem.cpp
    Engine/src/scene/system/RigidBodySystem.cpp
    Engine/src/scene/system/RenderingSystem.cpp
    Engine/src/scene/system/TransformSystem.cpp
)

set(RENDERER_SRC
    Engine/src/renderer/Colourpickingrenderer.cpp
    Engine/src/renderer/Material.cpp
    Engine/src/renderer/Mesh.cpp
    Engine/src/renderer/ObjLoader.cpp
    Engine/src/renderer/PrimitiveFactory.cpp
    Engine/src/renderer/Renderer.cpp
    Engine/src/renderer/ShaderProgram.cpp
    Engine/src/renderer/Texture.cpp
    Engine/src/renderer/ViewportFramebuffer.cpp
    Engine/src/renderer/stb_impl.cpp
)

set(CORE_SRC
    Engine/src/core/Application.cpp
    Engine/src/core/AssetManager.cpp
    Engine/src/core/Input.cpp
    Engine/src/core/Project.cpp
)

set(ECS_SRC
    Engine/src/ecs/EntityManager.cpp
    Engine/src/ecs/ComponentPoolRegistry.cpp
)

set(PROPERTY_SRC
    Engine/src/property/ComponentReflection.cpp
    Engine/src/property/PropertySerializer.cpp
)

set(PERSISTANCE_SRC
    Engine/src/persistance/Archive.cpp
    Engine/src/persistance/SceneSerializer.cpp
)

set(SCRIPTING_SRC
    Engine/src/scripting/ScriptCompiler.cpp
    Engine/src/scripting/ScriptLoader.cpp
)

# Editor sources
set(GUI_SRC
    Editor/gui/EditorCamera.cpp
    Editor/gui/EditorLayer.cpp
    Editor/gui/FileManagerPanel.cpp
    Editor/gui/HierarchyPanel.cpp
    Editor/gui/ImGuiLayer.cpp
)

set(CONTROLLER_SRC
    Editor/controller/AssetController.cpp
    Editor/controller/ComponentController.cpp
    Editor/controller/EditorSelectionController.cpp
    Editor/controller/EntityController.cpp
    Editor/controller/FileTypeController.cpp
    Editor/controller/ProjectController.cpp
    Editor/controller/SceneController.cpp
    Editor/controller/ScriptingController.cpp
    Editor/controller/TransformController.cpp
    Editor/controller/WindowController.cpp
)

set(PLATFORM_SRC
    Editor/platform/FileDialog.cpp
)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imguizmo/ImGuizmo.cpp
)

# Engine.dll (whole engine in a single .dll)
add_library(Engine SHARED
    ${PERSISTANCE_SRC}
    ${PROPERTY_SRC}
    ${CORE_SRC}
    ${ECS_SRC}
    ${PHYSICS_SRC}
    ${PHYSICS_BODY_SRC}
    ${SCENE_SRC}
    ${SCENE_COMPONENT_SRC}
    ${SCENE_SYSTEM_SRC}
    ${RENDERER_SRC}
    ${SCRIPTING_SRC}
    Engine/src/renderer/glad.c
)

set_target_properties(Engine PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(Engine PRIVATE
    ${PHYSX_DIR}/include
    external/nlohmann
)
target_include_directories(Engine PUBLIC
    Engine/include
    Engine/src
    external
    external/glm
    external/stb
)

target_link_libraries(Engine PUBLIC
    glfw
    OpenGL::GL
    ${PHYSX_LIB_DIR}/PhysX_64.lib
    ${PHYSX_LIB_DIR}/PhysXCommon_64.lib
    ${PHYSX_LIB_DIR}/PhysXFoundation_64.lib
    ${PHYSX_LIB_DIR}/PhysXExtensions_static_64.lib
    ${PHYSX_LIB_DIR}/PhysXCooking_64.lib
    ${PHYSX_LIB_DIR}/PhysXPvdSDK_static_64.lib
)

target_compile_definitions(Engine
    PRIVATE   GLAD_GLAPI_EXPORT GLAD_GLAPI_EXPORT_BUILD
    INTERFACE GLAD_GLAPI_EXPORT
)

# built-in shaders are read straight from the source tree, so edits don't need a copy step
# NOTE: this makes the exe depend on the source tree, a distributable build has to copy them next to the exe instead
target_compile_definitions(Engine PRIVATE
    VESTA_ENGINE_SHADER_DIR=u8"${CMAKE_SOURCE_DIR}/Engine/src/renderer/shaders/"
)

if (MSVC)
    target_compile_options(Engine PUBLIC /utf-8)
endif()

# Editor.lib
add_library(Editor STATIC
    ${GUI_SRC}
    ${CONTROLLER_SRC}
    ${PLATFORM_SRC}
    ${IMGUI_SRC}
)

target_include_directories(Editor PUBLIC
    Editor
    external/imgui
    external/imgui/backends
    external/imguizmo
)

target_link_libraries(Editor PUBLIC Engine)

# Editor (Sandbox.exe)
file(COPY Sandbox/icon.png DESTINATION ${CMAKE_BINARY_DIR})

add_executable(Sandbox Sandbox/src/main.cpp)

# opened when there is no project on the command line and no last opened project
target_compile_definitions(Sandbox PRIVATE
    VESTA_SAMPLE_PROJECT=u8"${CMAKE_SOURCE_DIR}/SampleProject"
)

if(WIN32)
    target_sources(Sandbox PRIVATE Sandbox/resources.rc)
    set_source_files_properties(Sandbox/resources.rc PROPERTIES
        COMPILE_FLAGS "/I\"${CMAKE_CURRENT_SOURCE_DIR}/Sandbox\"")
endif()

target_link_libraries(Sandbox PRIVATE Editor Engine)

set(PHYSX_DLL_NAMES PhysX_64.dll PhysXCommon_64.dll PhysXFoundation_64.dll PhysXCooking_64.dll)
foreach(DLL ${PHYSX_DLL_NAMES})
    add_custom_command(TARGET Sandbox POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PHYSX_DIR}/$<IF:$<CONFIG:Debug>,debug,release>/${DLL}
        $<TARGET_FILE_DIR:Sandbox>)
endforeach()

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:Sandbox>)

set(CLANG_CL ${CMAKE_SOURCE_DIR}/dependencies/compiler/clang-cl.exe)
set(LLDLINK  ${CMAKE_SOURCE_DIR}/dependencies/compiler/lld-link.exe)
if(EXISTS ${CLANG_CL})
    message(STATUS "Found clang-cl: ${CLANG_CL}")
else()
    message(WARNING "clang-cl not found at: ${CLANG_CL}")
endif()

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CLANG_CL} $<TARGET_FILE_DIR:Sandbox>/compiler/clang-cl.exe)
add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LLDLINK} $<TARGET_FILE_DIR:Sandbox>/compiler/lld-link.exe)

add_custom_command(TARGET Sandbox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Engine/include $<TARGET_FILE_DIR:Sandbox>/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/glm/glm $<TARGET_FILE_DIR:Sandbox>/include/glm
)

file(GLOB_RECURSE SCRIPT_HEADERS CONFIGURE_DEPENDS SampleProject/*.h)
list(FILTER SCRIPT_HEADERS EXCLUDE REGEX "/\\.[^/]*/")
if(SCRIPT_HEADERS)
    add_library(ScriptsIntelliSense EXCLUDE_FROM_ALL ${SCRIPT_HEADERS})
    set_target_properties(ScriptsIntelliSense PROPERTIES LINKER_LANGUAGE CXX)
    set_source_files_properties(${SCRIPT_HEADERS} PROPERTIES LANGUAGE CXX)
    target_include_directories(ScriptsIntelliSense PRIVATE
        Engine/include
        external/glm
    )
endif()
