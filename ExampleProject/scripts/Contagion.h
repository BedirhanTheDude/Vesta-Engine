#pragma once

#include <scripting/Script.h>
#include "Common.h"

// A contagion that jumps between dynamic rigid bodies
// An infected body glows sickly green, twitches (hops), and every so often infects bodies touching it
// by adding DemoContagion to them
BEGIN_SCRIPT(Contagion)
public:
    SERIALIZE(float, infectRadius, 1.7f);
    SERIALIZE(float, infectChance, 0.45f);
    SERIALIZE(float, checkInterval, 0.3f);
    SERIALIZE(float, sickTime, 7.0f);
    SERIALIZE(float, immuneTime, 4.0f);
    SERIALIZE(float, hopSpeed, 4.5f);

    void init(float sickSeconds) { sickTime = sickSeconds; }

    bool onAttach() override {
        if (Material* m = Demo::materialOf(entity)) {
            originalAlbedo = m->albedo;
            originalEmission = m->emission;
        }
        rng = Demo::Rng(0xC0FFEEu ^ (entity.getID() * 2246822519u));
        hopTimer = rng.range(0.3f, 1.5f);
        return true;
    }

    void onDetach() override {
        if (Material* m = Demo::materialOf(entity)) {
            m->albedo = originalAlbedo;
            m->emission = originalEmission;
        }
    }

    void onUpdate(float dt) override {
        age += dt;

        if (immune) {
            float k = 1.0f - glm::clamp(age / immuneTime, 0.0f, 1.0f);
            if (Material* m = Demo::materialOf(entity)) {
                m->albedo = glm::mix(originalAlbedo, glm::vec3(0.35f, 0.55f, 1.0f), k);
                m->emission = glm::mix(originalEmission, glm::vec3(0.05f, 0.12f, 0.35f), k);
            }
            if (age >= immuneTime)
                entity.removeComponent<Contagion>();
            return;
        }

        float pulse = 0.5f + 0.5f * std::sin(age * 7.0f);
        if (Material* m = Demo::materialOf(entity)) {
            m->albedo = glm::mix(originalAlbedo, glm::vec3(0.35f, 0.9f, 0.2f), 0.75f);
            m->emission = glm::vec3(0.15f, 0.6f, 0.05f) * (0.3f + pulse);
        }

        twitch(dt);

        spreadTimer += dt;
        if (spreadTimer >= checkInterval) {
            spreadTimer = 0.0f;
            spread();
        }

        if (age >= sickTime) {
            immune = true;
            age = 0.0f;
        }
    }

private:
    void twitch(float dt) {
        hopTimer -= dt;
        if (hopTimer > 0.0f) return;
        hopTimer = rng.range(0.8f, 2.0f);

        auto rb = entity.getComponent<RigidBodyComponent>();
        if (!rb || rb->getType() != Dynamic) return;

        glm::vec3 v = rb->getLinearVelocity();
        if (std::fabs(v.y) > 0.5f) return; // only hop when roughly at rest
        rb->setLinearVelocity(v + glm::vec3(rng.range(-1.5f, 1.5f), hopSpeed, rng.range(-1.5f, 1.5f)));
        rb->setAngularVelocity(glm::vec3(rng.range(-5, 5), rng.range(-5, 5), rng.range(-5, 5)));
    }

    void spread() {
        auto self = entity.getComponent<RigidBodyComponent>();
        if (!self) return;
        const glm::vec3 here = self->getWorldPosition();
        const float r2 = infectRadius * infectRadius;

        // ideally you would want a tag system that would tag entities with "infectable"
        // you would avoid iterating over all the entities
        for (const Entity& other : entity.getScene().getEntities()) {
            if (other == entity) continue;
            if (other.hasComponent<Contagion>()) continue; // sick, immune, or infected earlier this tick

            auto body = other.getComponent<RigidBodyComponent>();
            if (!body || body->getType() != Dynamic) continue;

            glm::vec3 d = body->getWorldPosition() - here;
            if (glm::dot(d, d) > r2) continue;
            if (rng.unit() > infectChance) continue;

            Entity target = other;
            target.addComponent<Contagion>();
        }
    }

    glm::vec3 originalAlbedo = glm::vec3(0.7f);
    glm::vec3 originalEmission = glm::vec3(0.0f);
    float age = 0.0f;
    float spreadTimer = 0.0f;
    float hopTimer = 1.0f;
    bool immune = false;
    Demo::Rng rng;
END_SCRIPT(Contagion)
