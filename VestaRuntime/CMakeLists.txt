# VestaRuntime.exe

add_executable(VestaRuntime main.cpp)

if(WIN32)
    target_sources(VestaRuntime PRIVATE resources/resources.rc)
    set_source_files_properties(resources/resources.rc PROPERTIES
        COMPILE_FLAGS "/I\"${CMAKE_CURRENT_SOURCE_DIR}\"")
endif()

target_link_libraries(VestaRuntime PRIVATE VestaEditor Vesta)

# Release is a native windowed app (no console)
if(MSVC)
    target_link_options(VestaRuntime PRIVATE
        "$<$<CONFIG:Release>:/SUBSYSTEM:WINDOWS;/ENTRY:mainCRTStartup>"
    )
endif()

# window icon, loaded relative to the working directory
file(COPY resources/icon.png DESTINATION ${VESTA_OUTPUT_DIR})

# runtime DLLs next to the exe
set(PHYSX_DLL_NAMES PhysX_64.dll PhysXCommon_64.dll PhysXFoundation_64.dll PhysXCooking_64.dll)
foreach(DLL ${PHYSX_DLL_NAMES})
    add_custom_command(TARGET VestaRuntime POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${PHYSX_LIB_DIR}/${DLL}
        $<TARGET_FILE_DIR:VestaRuntime>)
endforeach()

add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:glfw> $<TARGET_FILE_DIR:VestaRuntime>)

# bundled script toolchain: user scripts are compiled with these from the exe directory
set(CLANG_CL ${VESTA_DEPENDENCIES_DIR}/llvm/bin/clang-cl.exe)
set(LLDLINK  ${VESTA_DEPENDENCIES_DIR}/llvm/bin/lld-link.exe)
if(EXISTS ${CLANG_CL})
    message(STATUS "Found clang-cl: ${CLANG_CL}")
else()
    message(WARNING "clang-cl not found at: ${CLANG_CL}")
endif()

add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CLANG_CL} $<TARGET_FILE_DIR:VestaRuntime>/compiler/clang-cl.exe)
add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${LLDLINK} $<TARGET_FILE_DIR:VestaRuntime>/compiler/lld-link.exe)

# headers user scripts compile against
add_custom_command(TARGET VestaRuntime POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/Vesta/include $<TARGET_FILE_DIR:VestaRuntime>/include
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${VESTA_EXTERNAL_DIR}/glm/glm $<TARGET_FILE_DIR:VestaRuntime>/include/glm
)
