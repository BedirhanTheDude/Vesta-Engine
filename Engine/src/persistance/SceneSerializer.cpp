#include "SceneSerializer.h"

#include <fstream>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/CameraComponent.h>
#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

void SceneSerializer::save(const Scene& scene, bool temp) {
    if (scene.isPlaying) return;

    static std::string startSceneConfigName = "start.dat";

    std::string suffix = temp ? "_temp.vrea" : ".vrea";

    std::filesystem::path scenePath = std::filesystem::current_path() /
        "assets" / "scenes" / (scene.getSceneName() + suffix);

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
    arch.saveToFile(scenePath.string());

    if (!temp) {
        Archive cfg;
        cfg.set("last_scene", scene.getSceneName());
        cfg.saveToFile(startSceneConfigName);
    }
}

bool SceneSerializer::load(Scene& scene, bool temp) {
    scene.clear();
    scene.isPlaying = false;

    std::string suffix = temp ? "_temp.vrea" : ".vrea";

    std::filesystem::path scenePath = std::filesystem::current_path() /
        "assets" / "scenes" / (scene.getSceneName() + suffix);

    Archive arch;
    if (!arch.loadFromFile(scenePath.string())) return false;

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
