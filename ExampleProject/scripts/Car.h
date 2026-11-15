#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <core/AssetManager.h>
#include <renderer/LoadedModel.h>

// A car that drives in a circle
BEGIN_SCRIPT(Car)
public:
    SERIALIZE(std::string, modelPath, std::string("models/bmw.obj"));
    SERIALIZE(float, modelScale, 4.5f);
    SERIALIZE(float, yawOffset, -90.0f);
    SERIALIZE(float, groundOffset, 0.76f);
    SERIALIZE(glm::vec3, center, glm::vec3(0.0f));
    SERIALIZE(float, radius, 7.5f);
    SERIALIZE(float, speed, 7.0f);
    SERIALIZE(float, leanDegrees, 2.0f);
    SERIALIZE(bool, lights, true);
    SERIALIZE(float, headlightForward, 2.15f);
    SERIALIZE(float, headlightSide, 0.7f);
    SERIALIZE(float, headlightHeight, 0.7f);
    SERIALIZE(float, taillightBack, 2.2f);
    SERIALIZE(bool, pushesThings, true);
    SERIALIZE(glm::vec3, fallbackBoxSize, glm::vec3(1.95f, 1.4f, 4.5f));

    void onStart() override {
        if (loadModel()) entity.getTransform().setScale(glm::vec3(modelScale));

        angle = 0.0f;
        Scene& scene = entity.getScene();

        if (lights) {
            const glm::vec3 white(1.0f, 0.95f, 0.85f), red(1.0f, 0.08f, 0.04f);
            for (int i = 0; i < 2; ++i) {
                headlights[i] = scene.createEntity("Headlight");
                headlights[i].addComponent<PointLightComponent>(0.0f, 2.2f, 1.2f, 1.0f, 0.09f, 0.032f).setColor(white);

                taillights[i] = scene.createEntity("Taillight");
                taillights[i].addComponent<PointLightComponent>(0.0f, 0.9f, 0.3f, 1.0f, 0.35f, 0.44f).setColor(red);
            }
        }

        apply(poseAt(angle), 0.0f);

        if (pushesThings && !entity.hasComponent<RigidBodyComponent>()) {
            entity.addComponent<RigidBodyComponent>(Kinematic, nullptr, nullptr, true);
        }
    }

    void onUpdate(float dt) override {
        time += dt;
        if (radius > 0.01f) angle += (speed / radius) * dt;

        apply(poseAt(angle), time);
    }

private:
    struct Pose {
        glm::vec3 position;
        glm::vec3 forward;
        glm::vec3 outward;
        glm::quat heading;
    };

    Pose poseAt(float a) const {
        Pose p;
        p.outward = glm::vec3(std::cos(a), 0.0f, std::sin(a));
        p.position = center + p.outward * radius;

        p.forward = glm::vec3(-std::sin(a), 0.0f, std::cos(a)) * (speed >= 0.0f ? 1.0f : -1.0f);
        p.heading = Demo::lookRotation(p.forward);
        return p;
    }

    void apply(const Pose& p, float t) {
        const glm::vec3 up(0.0f, 1.0f, 0.0f);

        // body roll
        float lateral = (radius > 0.01f) ? (speed * speed / radius) / 9.81f : 0.0f;
        float roll = glm::radians(glm::clamp(leanDegrees * lateral * 3.0f, 0.0f, leanDegrees * 2.0f));
        glm::quat rollQ = glm::angleAxis(roll, glm::normalize(glm::cross(up, p.outward)));

        float bob = std::sin(t * 9.0f) * 0.012f + std::sin(t * 3.7f) * 0.008f;

        TransformComponent tr = entity.getTransform();
        tr.setPosition(p.position + up * (groundOffset + bob));
        tr.setRotation(rollQ * p.heading * glm::angleAxis(glm::radians(yawOffset), up));

        // lights follow in world space
        // could also make them children and not have to worry about this
        glm::vec3 right = glm::cross(p.forward, up);
        for (int i = 0; i < 2; ++i) {
            float side = (i == 0) ? -1.0f : 1.0f;
            if (headlights[i].isAlive())
                headlights[i].getTransform().setPosition(p.position + p.forward * headlightForward
                    + right * (headlightSide * side) + up * headlightHeight);
            if (taillights[i].isAlive())
                taillights[i].getTransform().setPosition(p.position - p.forward * taillightBack
                    + right * (headlightSide * side) + up * headlightHeight);
        }
    }

    bool loadModel() {
        LoadedModel model;
        if (!modelPath.empty()) model = AssetManager::getModel(modelPath);

        if (!model.mesh) {
            if (auto mesh = entity.getComponent<MeshComponent>()) mesh->setPrimitive(Demo::PrimCube);
            else entity.addComponent<MeshComponent>().setPrimitive(Demo::PrimCube);
            if (!entity.hasComponent<MaterialComponent>()) entity.addComponent<MaterialComponent>();
            modelScale = 1.0f;
            yawOffset = 0.0f;
            groundOffset = fallbackBoxSize.y * 0.5f;
            entity.getTransform().setScale(fallbackBoxSize);
            return false;
        }

        if (auto mesh = entity.getComponent<MeshComponent>()) mesh->setMesh(model.mesh);
        else entity.addComponent<MeshComponent>(model.mesh);

        if (auto mats = entity.getComponent<MaterialComponent>()) {
            if (!model.materials.empty()) mats->setMaterials(model.materials);
        }
        else if (!model.materials.empty()) entity.addComponent<MaterialComponent>(model.materials);
        else entity.addComponent<MaterialComponent>();
        return true;
    }

    Entity headlights[2];
    Entity taillights[2];
    float angle = 0.0f;
    float time = 0.0f;

END_SCRIPT(Car)
