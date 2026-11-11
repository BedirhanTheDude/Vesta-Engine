cmake_minimum_required(VERSION 3.16)
project(GameEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(PHYSX_DIR ${CMAKE_SOURCE_DIR}/dependencies/physx)
set(PHYSX_LIB_DIR ${PHYSX_DIR}/$<IF:$<CONFIG:Debug>,debug,release>)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(external/glfw)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(external/lunasvg)

find_package(OpenGL REQUIRED)

# Vesta sources
set(PHYSICS_SRC
    Vesta/src/physics/CollisionShape.cpp
    Vesta/src/physics/PhysicsMaterial.cpp
    Vesta/src/physics/PhysicsWorld.cpp
)

set(PHYSICS_BODY_SRC
    Vesta/src/physics/body/DynamicPhysicsBody.cpp
    Vesta/src/physics/body/PhysicsBody.cpp
    Vesta/src/physics/body/StaticPhysicsBody.cpp
)

set(SCENE_SRC
    Vesta/src/scene/Entity.cpp
    Vesta/src/scene/Scene.cpp
)

set(SCENE_COMPONENT_SRC
    Vesta/src/scene/components/BehaviourComponent.cpp
    Vesta/src/scene/components/CameraComponent.cpp
    Vesta/src/scene/components/ComponentFactory.cpp
    Vesta/src/scene/components/DirectionalLightComponent.cpp
    Vesta/src/scene/components/MaterialComponent.cpp
    Vesta/src/scene/components/MeshComponent.cpp
    Vesta/src/scene/components/PointLightComponent.cpp
    Vesta/src/scene/components/RigidBodyComponent.cpp
    Vesta/src/scene/components/TransformComponent.cpp
)

set(SCENE_SYSTEM_SRC
    Vesta/src/scene/system/CameraSystem.cpp
    Vesta/src/scene/system/DirectionalLightSystem.cpp
    Vesta/src/scene/system/MaterialSystem.cpp
    Vesta/src/scene/system/MeshSystem.cpp
    Vesta/src/scene/system/PointLightSystem.cpp
    Vesta/src/scene/system/RigidBodySystem.cpp
    Vesta/src/scene/system/RenderingSystem.cpp
    Vesta/src/scene/system/ScriptSystem.cpp
    Vesta/src/scene/system/TransformSystem.cpp
)

set(RENDERER_SRC
    Vesta/src/renderer/Colourpickingrenderer.cpp
    Vesta/src/renderer/Material.cpp
    Vesta/src/renderer/Mesh.cpp
    Vesta/src/renderer/ObjLoader.cpp
    Vesta/src/renderer/PrimitiveFactory.cpp
    Vesta/src/renderer/Renderer.cpp
    Vesta/src/renderer/ShaderProgram.cpp
    Vesta/src/renderer/Texture.cpp
    Vesta/src/renderer/ViewportFramebuffer.cpp
    Vesta/src/renderer/stb_impl.cpp
)

set(CORE_SRC
    Vesta/src/core/Application.cpp
    Vesta/src/core/AssetManager.cpp
    Vesta/src/core/Input.cpp
    Vesta/src/core/Project.cpp
)

set(ECS_SRC
    Vesta/src/ecs/EntityManager.cpp
    Vesta/src/ecs/ComponentPoolRegistry.cpp
    Vesta/src/ecs/ScriptPool.cpp
)

set(PROPERTY_SRC
    Vesta/src/property/ComponentReflection.cpp
    Vesta/src/property/PropertySerializer.cpp
)

set(PERSISTANCE_SRC
    Vesta/src/persistance/Archive.cpp
    Vesta/src/persistance/SceneSerializer.cpp
)

set(SCRIPTING_SRC
    Vesta/src/scripting/ScriptCache.cpp
    Vesta/src/scripting/ScriptCompiler.cpp
    Vesta/src/scripting/ScriptLoader.cpp
)

set(UTILITY_SRC
    Vesta/src/utility/Hash.cpp
)

# VestaEditor sources
set(GUI_SRC
    VestaEditor/src/gui/EditorCamera.cpp
    VestaEditor/src/gui/EditorLayer.cpp
    VestaEditor/src/gui/FileManagerPanel.cpp
    VestaEditor/src/gui/HierarchyPanel.cpp
    VestaEditor/src/gui/ImGuiLayer.cpp
    VestaEditor/src/gui/Thumbnail.cpp
    VestaEditor/src/gui/SvgPixels.cpp
)

set(EDITOR_APP_SRC
    VestaEditor/src/EditorApp.cpp
)

set(CONTROLLER_SRC
    VestaEditor/src/controller/AssetController.cpp
    VestaEditor/src/controller/ComponentController.cpp
    VestaEditor/src/controller/EditorSelectionController.cpp
    VestaEditor/src/controller/EntityController.cpp
    VestaEditor/src/controller/FileTypeController.cpp
    VestaEditor/src/controller/ProjectController.cpp
    VestaEditor/src/controller/SceneController.cpp
    VestaEditor/src/controller/ScriptingController.cpp
    VestaEditor/src/controller/TransformController.cpp
    VestaEditor/src/controller/WindowController.cpp
)

set(PLATFORM_SRC
    VestaEditor/src/platform/FileDialog.cpp
)

set(IMGUI_SRC
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
    external/imguizmo/src/ImGuizmo.cpp
)

# Vesta.dll (whole engine in a single .dll)
add_library(Vesta SHARED
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
    ${UTILITY_SRC}
    Vesta/src/renderer/glad.c
)

set_target_properties(Vesta PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(Vesta PRIVATE
    ${PHYSX_DIR}/include
)

target_include_directories(Vesta PUBLIC
    Vesta/include
    Vesta/src
    external
    external/glm
    external/stb
)

target_link_libraries(Vesta PUBLIC
    glfw
    OpenGL::GL
    ${PHYSX_LIB_DIR}/PhysX_64.lib
    ${PHYSX_LIB_DIR}/PhysXCommon_64.lib
    ${PHYSX_LIB_DIR}/PhysXFoundation_64.lib
    ${PHYSX_LIB_DIR}/PhysXExtensions_static_64.lib
    ${PHYSX_LIB_DIR}/PhysXCooking_64.lib
    ${PHYSX_LIB_DIR}/PhysXPvdSDK_static_64.lib
)

target_compile_definitions(Vesta
    PRIVATE   GLAD_GLAPI_EXPORT GLAD_GLAPI_EXPORT_BUILD
    INTERFACE GLAD_GLAPI_EXPORT
)

# built-in shaders are read straight from the source tree, so edits don't need a copy step
# NOTE: this makes the exe depend on the source tree, a distributable build has to copy them next to the exe instead
target_compile_definitions(Vesta PRIVATE
    VESTA_ENGINE_SHADER_DIR=u8"${CMAKE_SOURCE_DIR}/Vesta/src/renderer/shaders/"
)

if (MSVC)
    target_compile_options(Vesta PUBLIC /utf-8)
endif()

# Editor.lib
add_library(VestaEditor STATIC
    ${EDITOR_APP_SRC}
    ${GUI_SRC}
    ${CONTROLLER_SRC}
    ${PLATFORM_SRC}
    ${IMGUI_SRC}
)

target_include_directories(VestaEditor PUBLIC
    VestaEditor/src
    external/imgui
    external/imgui/backends
    external/imguizmo
    external/lunasvg
)

target_link_libraries(VestaEditor PUBLIC Vesta)
target_link_libraries(VestaEditor PRIVATE lunasvg)

# Editor Resources
file(COPY VestaEditor/resources DESTINATION ${CMAKE_BINARY_DIR})
file(COPY VestaRuntime/icon.png DESTINATION ${CMAKE_BINARY_DIR})

add_executable(VestaRuntime VestaRuntime/main.cpp)

if(WIN32)
    target_sources(VestaRuntime PRIVATE VestaRuntime/resources.rc)
    set_source_files_properties(VestaRuntime/resources.rc PROPERTIES
        COMPILE_FLAGS "/I\"${CMAKE_CURRENT_SOURCE_DIR}/VestaRuntime\"")
endif()

target_link_libraries(VestaRuntime PRIVATE VestaEditor Vesta)

# Release is a native windowed app (no console)
if(MSVC)
    target_link_options(VestaRuntime PRIVATE
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS;/ENTRY:mainCRTStartup>"
    )
endif()

set(PHYSX_DLL_NAMES PhysX_64.dll PhysXCommon_64.dll PhysXFoundation_64.dll PhysXCooking_64.dll)
foreach(DLL ${PHYSX_DLL_NAMES})
    add_custom_command(TARGET VestaRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PHYSX_DIR}/$<IF:$<CONFIG:Debug>,debug,release>/${DLL}
        $<TARGET_FILE_DIR:VestaRuntime>)
endforeach()

add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:VestaRuntime>)

set(CLANG_CL ${CMAKE_SOURCE_DIR}/dependencies/llvm/bin/clang-cl.exe)
set(LLDLINK  ${CMAKE_SOURCE_DIR}/dependencies/llvm/bin/lld-link.exe)
if(EXISTS ${CLANG_CL})
    message(STATUS "Found clang-cl: ${CLANG_CL}")
else()
    message(WARNING "clang-cl not found at: ${CLANG_CL}")
endif()

add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CLANG_CL} $<TARGET_FILE_DIR:VestaRuntime>/compiler/clang-cl.exe)
add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LLDLINK} $<TARGET_FILE_DIR:VestaRuntime>/compiler/lld-link.exe)

add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Vesta/include $<TARGET_FILE_DIR:VestaRuntime>/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/external/glm/glm $<TARGET_FILE_DIR:VestaRuntime>/include/glm
)

file(GLOB_RECURSE SCRIPT_HEADERS CONFIGURE_DEPENDS SampleProject/*.h)
list(FILTER SCRIPT_HEADERS EXCLUDE REGEX "/\\.[^/]*/")
if(SCRIPT_HEADERS)
    add_library(ScriptsIntelliSense EXCLUDE_FROM_ALL ${SCRIPT_HEADERS})
    set_target_properties(ScriptsIntelliSense PROPERTIES LINKER_LANGUAGE CXX)
    set_source_files_properties(${SCRIPT_HEADERS} PROPERTIES LANGUAGE CXX)
    target_include_directories(ScriptsIntelliSense PRIVATE
        Vesta/include
        external/glm
    )
endif()
