# Vesta.dll (whole engine in a single .dll)

set(PHYSICS_SRC
    src/physics/CollisionShape.cpp
    src/physics/PhysicsMaterial.cpp
    src/physics/PhysicsWorld.cpp
)

set(PHYSICS_BODY_SRC
    src/physics/body/DynamicPhysicsBody.cpp
    src/physics/body/PhysicsBody.cpp
    src/physics/body/StaticPhysicsBody.cpp
)

set(SCENE_SRC
    src/scene/Entity.cpp
    src/scene/Scene.cpp
)

set(SCENE_COMPONENT_SRC
    src/scene/components/BehaviourComponent.cpp
    src/scene/components/CameraComponent.cpp
    src/scene/components/ComponentFactory.cpp
    src/scene/components/DirectionalLightComponent.cpp
    src/scene/components/MaterialComponent.cpp
    src/scene/components/MeshComponent.cpp
    src/scene/components/PointLightComponent.cpp
    src/scene/components/RigidBodyComponent.cpp
    src/scene/components/TransformComponent.cpp
)

set(SCENE_SYSTEM_SRC
    src/scene/system/CameraSystem.cpp
    src/scene/system/DirectionalLightSystem.cpp
    src/scene/system/MaterialSystem.cpp
    src/scene/system/MeshSystem.cpp
    src/scene/system/PointLightSystem.cpp
    src/scene/system/RigidBodySystem.cpp
    src/scene/system/RenderingSystem.cpp
    src/scene/system/ScriptSystem.cpp
    src/scene/system/TransformSystem.cpp
)

set(RENDERER_SRC
    src/renderer/Colourpickingrenderer.cpp
    src/renderer/Material.cpp
    src/renderer/Mesh.cpp
    src/renderer/ObjLoader.cpp
    src/renderer/PrimitiveFactory.cpp
    src/renderer/Renderer.cpp
    src/renderer/ShaderProgram.cpp
    src/renderer/Texture.cpp
    src/renderer/ViewportFramebuffer.cpp
    src/renderer/stb_impl.cpp
)

set(CORE_SRC
    src/core/Application.cpp
    src/core/AssetManager.cpp
    src/core/Input.cpp
    src/core/Project.cpp
)

set(ECS_SRC
    src/ecs/EntityManager.cpp
    src/ecs/ComponentPoolRegistry.cpp
    src/ecs/ScriptPool.cpp
)

set(PROPERTY_SRC
    src/property/ComponentReflection.cpp
    src/property/PropertySerializer.cpp
)

set(PERSISTENCE_SRC
    src/persistence/Archive.cpp
    src/persistence/SceneSerializer.cpp
)

set(SCRIPTING_SRC
    src/scripting/ScriptCache.cpp
    src/scripting/ScriptCompiler.cpp
    src/scripting/ScriptLoader.cpp
)

set(UTILITY_SRC
    src/utility/Hash.cpp
)

add_library(Vesta SHARED
    ${PERSISTENCE_SRC}
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
    src/renderer/glad.c
)

set_target_properties(Vesta PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)

target_include_directories(Vesta PRIVATE
    ${PHYSX_DIR}/include
)

target_include_directories(Vesta PUBLIC
    include
    src
    ${VESTA_EXTERNAL_DIR}
    ${VESTA_EXTERNAL_DIR}/glm
    ${VESTA_EXTERNAL_DIR}/stb
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
    VESTA_ENGINE_SHADER_DIR=u8"${CMAKE_CURRENT_SOURCE_DIR}/src/renderer/shaders/"
)

if(MSVC)
    target_compile_options(Vesta PUBLIC /utf-8)
endif()
