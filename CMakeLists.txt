cmake_minimum_required(VERSION 3.26)
project(VestaEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(VESTA_EXTERNAL_DIR     ${CMAKE_SOURCE_DIR}/external)
set(VESTA_DEPENDENCIES_DIR ${CMAKE_SOURCE_DIR}/dependencies)

set(PHYSX_DIR ${VESTA_DEPENDENCIES_DIR}/physx)
set(PHYSX_LIB_DIR ${PHYSX_DIR}/$<IF:$<CONFIG:Debug>,debug,release>)

set(BUILD_SHARED_LIBS ON)
add_subdirectory(external/glfw)
set(BUILD_SHARED_LIBS OFF)

add_subdirectory(external/lunasvg)

find_package(OpenGL REQUIRED)

set(VESTA_OUTPUT_DIR ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${VESTA_OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${VESTA_OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${VESTA_OUTPUT_DIR})

add_subdirectory(Vesta)
add_subdirectory(VestaEditor)
add_subdirectory(VestaRuntime)

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
