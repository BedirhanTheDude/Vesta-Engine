#pragma once

#include <scripting/Script.h>
#include "Common.h"
#include "Firework.h"

// Explosive charge for a dynamic rigid body
BEGIN_SCRIPT(Explosive)
public:
    SERIALIZE(float, power, 90.0f);
    SERIALIZE(float, radius, 7.0f);
    SERIALIZE(float, fuse, 6.0f);
    SERIALIZE(float, armDelay, 0.15f);
    SERIALIZE(float, impactThreshold, 6.0f);

    void init(float newPower) { power = newPower; }

    void onStart() override {
        if (!entity.hasComponent<PointLightComponent>())
            entity.addComponent<PointLightComponent>(0.0f, 0.0f, 0.0f, 1.0f, 0.35f, 0.44f)
                .setColor(glm::vec3(1.0f, 0.2f, 0.05f));
    }

    void onUpdate(float dt) override {
        if (done) return;
        age += dt;

        auto rb = entity.getComponent<RigidBodyComponent>();
        float speed = rb ? glm::length(rb->getLinearVelocity()) : 0.0f;

        // blink faster as the fuse burns down
        float rate = 4.0f + 20.0f * glm::clamp(age / fuse, 0.0f, 1.0f);
        float blink = 0.5f + 0.5f * std::sin(age * rate);
        if (Material* m = Demo::materialOf(entity))
            m->emission = glm::vec3(1.0f, 0.2f, 0.05f) * blink * 1.5f;
        if (auto light = entity.getComponent<PointLightComponent>()) {
            light->setDiffuseStrength(1.5f * blink);
            light->setSpecularStrength(0.8f * blink);
        }

        bool hit = age > armDelay && (lastSpeed - speed) > impactThreshold;
        lastSpeed = speed;

        if (hit || age >= fuse) detonate();
    }

private:
    void detonate() {
        done = true;
        Scene& scene = entity.getScene();
        const glm::vec3 centre = entity.getTransform().getWorldPosition();

        for (const Entity& other : scene.getEntities()) {
            if (other == entity) continue;
            auto body = other.getComponent<RigidBodyComponent>();
            if (!body || body->getType() != Dynamic) continue;

            glm::vec3 d = body->getWorldPosition() - centre;
            float dist = glm::length(d);
            if (dist > radius || dist < 1e-4f) continue;

            float falloff = 1.0f - dist / radius;

            // a bit of upward kick makes blasts look much better than a flat push
            glm::vec3 dir = glm::normalize(d + glm::vec3(0.0f, 0.6f, 0.0f));
            body->addImpulse(dir * power * falloff * falloff);
            body->setAngularVelocity(body->getAngularVelocity() + glm::vec3(d.z, 0.0f, -d.x) * falloff);
        }

        // really need a particle system for these kinds of things
        glm::vec3 c(1.0f, 0.55f, 0.15f);
        Entity flash = Demo::spawnPrimitive(scene, "Blast", Demo::PrimSphere, centre, glm::vec3(0.6f), c);
        flash.addComponent<PointLightComponent>(0.0f, 4.0f, 2.0f, 1.0f, 0.09f, 0.032f).setColor(c);
        flash.addComponent<Firework>(Firework::Rocket, 0, glm::vec3(0.0f), c, 0.0f);

        scene.removeEntity(entity);
    }

    float age = 0.0f;
    float lastSpeed = 0.0f;
    bool done = false;
END_SCRIPT(Explosive)
