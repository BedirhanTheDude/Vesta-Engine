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
file(GLOB PHYSICS_SRC         CONFIGURE_DEPENDS Engine/src/physics/*.cpp)
file(GLOB PHYSICS_BODY_SRC    CONFIGURE_DEPENDS Engine/src/physics/body/*.cpp)
file(GLOB SCENE_SRC           CONFIGURE_DEPENDS Engine/src/scene/*.cpp)
file(GLOB SCENE_COMPONENT_SRC CONFIGURE_DEPENDS Engine/src/scene/components/*.cpp)
file(GLOB RENDERER_SRC        CONFIGURE_DEPENDS Engine/src/renderer/*.cpp)
file(GLOB CORE_SRC            CONFIGURE_DEPENDS Engine/src/core/*.cpp)
file(GLOB PROPERTY_SRC        CONFIGURE_DEPENDS Engine/src/property/*.cpp)
file(GLOB PERSISTANCE_SRC     CONFIGURE_DEPENDS Engine/src/persistance/*.cpp)
file(GLOB SCRIPTING_SRC       CONFIGURE_DEPENDS Engine/src/scripting/*.cpp)

# Editor sources
file(GLOB GUI_SRC             CONFIGURE_DEPENDS Editor/gui/*.cpp)
file(GLOB CONTROLLER_SRC      CONFIGURE_DEPENDS Editor/controller/*.cpp)

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
    ${PHYSICS_SRC}
    ${PHYSICS_BODY_SRC}
    ${SCENE_SRC}
    ${SCENE_COMPONENT_SRC}
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

target_compile_definitions(Engine PUBLIC
    SHADER_PATH=u8"${CMAKE_SOURCE_DIR}/assets/shaders/"
    MODEL_PATH=u8"${CMAKE_SOURCE_DIR}/assets/models/"
    SCENES_PATH=u8"${CMAKE_SOURCE_DIR}/assets/scenes/"
    ASSETS_PATH=u8"${CMAKE_SOURCE_DIR}/assets/"
    SCRIPTS_PATH="${CMAKE_SOURCE_DIR}/Sandbox/scripts/"
)

if (MSVC)
    target_compile_options(Engine PUBLIC /utf-8)
endif()

# Editor.lib
add_library(Editor STATIC
    ${GUI_SRC}
    ${CONTROLLER_SRC}
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
file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Sandbox/icon.png DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Sandbox/src/scripts DESTINATION ${CMAKE_BINARY_DIR})

add_executable(Sandbox Sandbox/src/main.cpp)

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

file(GLOB_RECURSE SCRIPT_HEADERS CONFIGURE_DEPENDS Sandbox/src/scripts/*.h)
if(SCRIPT_HEADERS)
    add_library(ScriptsIntelliSense EXCLUDE_FROM_ALL ${SCRIPT_HEADERS})
    set_target_properties(ScriptsIntelliSense PROPERTIES LINKER_LANGUAGE CXX)
    set_source_files_properties(${SCRIPT_HEADERS} PROPERTIES LANGUAGE CXX)
    target_include_directories(ScriptsIntelliSense PRIVATE
        Engine/include
        external/glm
    )
endif()
