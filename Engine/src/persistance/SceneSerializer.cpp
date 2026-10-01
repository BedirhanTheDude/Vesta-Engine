#include "SceneSerializer.h"

#include <filesystem>
#include <iostream>
#include <system_error>

#include <glm/gtc/type_ptr.hpp>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>
#include <core/Project.h>

static bool getSceneFilePath(const Scene& scene, bool temp, std::filesystem::path& outPath) {
    if (!Project::isOpen()) {
        std::cerr << "[Error] No project is open\n";
        return false;
    }

    if (temp) {
        outPath = Project::getCacheDirectory() / (std::string("temp") + Project::sceneExtension);
        return true;
    }

    if (!Project::resolve(scene.getScenePath(), outPath)) {
        std::cerr << "[Error] Scene \"" << scene.getSceneName() << "\" has no valid path inside the project: \""
            << scene.getScenePath() << "\"\n";
        return false;
    }

    return true;
}

bool SceneSerializer::save(const Scene& scene, bool temp) {
    if (scene.isPlaying) return false;

    std::filesystem::path scenePath;
    if (!getSceneFilePath(scene, temp, scenePath)) return false;

    Archive arch;
    Archive sceneArch;

    Entity activeCamera = scene.getActiveCameraEntity();
    if (activeCamera.isValid())
        sceneArch.set("activeCamera", activeCamera.getName());

    for (const Entity& entity : scene.getEntities()) {
        Archive ej;
        ej.set("name", entity.getName());

        TransformComponent transform = entity.getTransform();

        Entity parent = transform.getParent();
        ej.set("parent", parent.isAlive() ? parent.getName() : std::string(""));

        // world space, the parent is re-applied (keeping the world transform) once every entity exists
        Archive transformArch;
        transformArch.set("position", transform.getWorldPosition());
        transformArch.set("rotation", transform.getWorldRotationQuat());
        transformArch.set("scale", transform.getWorldScale());
        ej.set("transform", std::move(transformArch));

        ComponentFactory::serializeEntity(entity, ej);

        sceneArch.append("entities", std::move(ej));
    }

    arch.set("scene", std::move(sceneArch));

    std::error_code ec;
    std::filesystem::create_directories(scenePath.parent_path(), ec);

    if (!arch.saveToFile(scenePath)) {
        std::cerr << "[Error] Could not write scene file " << scenePath << '\n';
        return false;
    }

    if (!temp)
        Project::setStartupScene(scene.getScenePath());

    return true;
}

bool SceneSerializer::load(Scene& scene, bool temp) {
    std::filesystem::path scenePath;
    if (!getSceneFilePath(scene, temp, scenePath)) return false;

    Archive arch;
    if (!arch.loadFromFile(scenePath)) {
        std::cerr << "[Error] Could not read scene file " << scenePath << '\n';
        return false;
    }

    scene.clear();

    Archive sceneArch = arch.get("scene");

    size_t entityCount = sceneArch.size("entities");

    // create entities, transforms, components
    for (size_t i = 0; i < entityCount; i++) {
        Archive ej = sceneArch.at("entities", i);

        std::string name;
        ej.get("name", name);
        Entity entity = scene.createEntityImmediate(name);

        Archive transformArch = ej.get("transform");
        glm::vec3 pos(0.0f), scl(1.0f);
        glm::quat rot(1.0f, 0.0f, 0.0f, 0.0f);
        transformArch.get("position", pos);
        transformArch.get("rotation", rot);
        transformArch.get("scale", scl);

        TransformComponent transform = entity.getTransform();
        transform.setPosition(pos);
        transform.setRotation(rot);
        transform.setScale(scl);

        size_t compCount = ej.size("components");
        for (size_t c = 0; c < compCount; c++) {
            Archive cj = ej.at("components", c);
            std::string type;
            cj.get("type", type);
            ComponentFactory::create(type, entity, cj);
        }
    }

    // resolve parents
    for (size_t i = 0; i < entityCount; i++) {
        Archive ej = sceneArch.at("entities", i);

        std::string parentName;
        if (ej.get("parent", parentName) && !parentName.empty()) {
            std::string childName;
            ej.get("name", childName);

            Entity child = scene.findEntity(childName);
            Entity parent = scene.findEntity(parentName);

            if (child.isValid() && parent.isValid())
                child.getTransform().setParent(parent);
        }
    }

    std::string camName;
    if (sceneArch.get("activeCamera", camName)) {
        Entity camEntity = scene.findEntity(camName);
        if (camEntity.isValid() && camEntity.hasComponent<CameraComponent>())
            scene.setActiveCamera(camEntity);
    }

    return true;
}
