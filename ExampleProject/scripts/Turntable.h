#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <core/AssetManager.h>
#include <renderer/LoadedModel.h>
#include <renderer/Mesh.h>

// Showcase turntable: optionally loads an .obj model (with its .mtl materials) onto this
// entity, then slowly spins it and lets it float up and down
BEGIN_SCRIPT(Turntable)
public:
    SERIALIZE(std::string, modelPath, std::string(""));
    SERIALIZE(float, modelScale, 1.0f);
    SERIALIZE(float, degreesPerSecond, 25.0f);
    SERIALIZE(float, floatHeight, 0.25f);
    SERIALIZE(float, floatSpeed, 1.2f);

    void onStart() override {
        TransformComponent t = entity.getTransform();
        basePos = t.getPosition();

        if (!modelPath.empty()) loadModel();
        t.setScale(glm::vec3(modelScale));
    }

    void onUpdate(float dt) override {
        time += dt;
        TransformComponent t = entity.getTransform();
        t.rotateAroundAxis(glm::vec3(0, 1, 0), degreesPerSecond * dt);
        t.setPosition(basePos + glm::vec3(0.0f, std::sin(time * floatSpeed) * floatHeight, 0.0f));
    }

private:
    void loadModel() {
        LoadedModel model = AssetManager::getModel(modelPath);
        if (!model.mesh) return;

        if (auto mesh = entity.getComponent<MeshComponent>()) mesh->setMesh(model.mesh);
        else entity.addComponent<MeshComponent>(model.mesh);

        if (auto mats = entity.getComponent<MaterialComponent>()) {
            if (!model.materials.empty()) mats->setMaterials(model.materials);
        }
        else if (!model.materials.empty()) entity.addComponent<MaterialComponent>(model.materials);
        else entity.addComponent<MaterialComponent>();
    }

    glm::vec3 basePos = glm::vec3(0.0f);
    float time = 0.0f;
END_SCRIPT(Turntable)
