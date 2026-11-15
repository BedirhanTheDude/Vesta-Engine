#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <core/Input.h>

// Self-replicating fireworks
// Press L to fire a big salvo
BEGIN_SCRIPT(Firework)
public:
    enum Role : int { Launcher = 0, Rocket = 1, Spark = 2 };

    SERIALIZE(int, role, 0);
    SERIALIZE(float, launchInterval, 1.4f);
    SERIALIZE(float, launchSpread, 7.0f);
    SERIALIZE(int, sparksPerBurst, 60);
    SERIALIZE(float, crackleChance, 0.15f);
    SERIALIZE(float, brightness, 1.0f);
    SERIALIZE(float, flashIntensity, 1.0f);
    SERIALIZE(float, flashDuration, 1.1f);
    SERIALIZE(float, sparkSize, 0.24f);

    void init(int newRole, int newGeneration, const glm::vec3& velocity, const glm::vec3& color, float life) {
        role = newRole;
        generation = newGeneration;
        vel = velocity;
        col = color;
        lifetime = life;
    }

    void inheritSettings(Firework& child) const {
        child.sparksPerBurst = sparksPerBurst;
        child.crackleChance = crackleChance;
        child.brightness = brightness;
        child.flashIntensity = flashIntensity;
        child.flashDuration = flashDuration;
        child.sparkSize = sparkSize;
    }

    void onStart() override {
        rng = Demo::Rng(0xF1BEu ^ (entity.getID() * 2654435761u));
        pos = entity.getTransform().getWorldPosition();
        baseScale = entity.getTransform().getScale().x;
    }

    void onUpdate(float dt) override {
        if (dead) return;
        age += dt;

        switch (role) {
        case Launcher: updateLauncher(dt); break;
        case Rocket:   updateRocket(dt);   break;
        case Spark:    updateSpark(dt);    break;
        default: break;
        }
    }

private:
    // launcher
    void updateLauncher(float dt) {
        timer += dt;

        if (Input::isKeyPressed(Key::L)) salvo = 8;

        float interval = salvo > 0 ? 0.12f : launchInterval;
        if (timer < interval) return;
        timer = 0.0f;
        if (salvo > 0) --salvo;

        glm::vec3 origin = entity.getTransform().getWorldPosition();
        glm::vec3 start = origin + glm::vec3(rng.range(-launchSpread, launchSpread), 0.3f, rng.range(-2.0f, 2.0f));
        glm::vec3 v(rng.range(-1.5f, 1.5f), rng.range(17.0f, 22.0f), rng.range(-1.5f, 1.5f));
        glm::vec3 c = Demo::hsv(rng.unit(), rng.range(0.6f, 0.9f), 1.0f);

        Scene& scene = entity.getScene();
        Entity rocket = Demo::spawnPrimitive(scene, "Rocket", Demo::PrimSphere, start, glm::vec3(0.3f, 0.6f, 0.3f), c);
        if (Material* m = Demo::materialOf(rocket)) m->emission = (glm::vec3(1.0f) * 0.6f + c * 1.5f) * brightness;
        rocket.addComponent<PointLightComponent>(0.0f, 2.5f * brightness, 1.2f * brightness, 1.0f, 0.09f, 0.032f).setColor(c);
        inheritSettings(rocket.addComponent<Firework>(Rocket, 0, v, c, rng.range(1.1f, 1.5f)));
    }

    // rocket
    void updateRocket(float dt) {
        if (!burst) {
            vel.y -= 9.0f * dt;
            pos += vel * dt;
            TransformComponent t = entity.getTransform();
            t.setPosition(pos);
            t.setRotation(Demo::lookRotation(vel));

            if (age >= lifetime || vel.y < 1.0f) explode();
            return;
        }

        // after the burst the rocket entity lingers as a fading flash of light
        float k = 1.0f - glm::clamp((age - burstAge) / (std::max)(flashDuration, 0.05f), 0.0f, 1.0f);
        float flash = k * std::sqrt(k);
        if (auto light = entity.getComponent<PointLightComponent>()) {
            light->setDiffuseStrength(flashIntensity * brightness * flash);
            light->setSpecularStrength(flashIntensity * brightness * 0.5f * flash);
        }
        if (Material* m = Demo::materialOf(entity)) m->emission = (glm::vec3(1.0f) + col) * 2.0f * k * brightness;
        entity.getTransform().setScale(glm::vec3(1.2f * k + 0.001f));

        if (k <= 0.0f) die();
    }

    void explode() {
        burst = true;
        burstAge = age;

        if (auto light = entity.getComponent<PointLightComponent>()) {
            light->setLinear(0.022f);
            light->setQuadratic(0.0019f);
            light->setDiffuseStrength(flashIntensity * brightness);
            light->setSpecularStrength(flashIntensity * brightness * 0.5f);
        }

        Scene& scene = entity.getScene();
        bool twoTone = rng.unit() < 0.4f;
        glm::vec3 second = Demo::hsv(rng.unit(), 0.8f, 1.0f);
        float speed = rng.range(6.5f, 9.0f);

        // golden-spiral directions -> an even sphere of sparks
        const float golden = glm::pi<float>() * (3.0f - std::sqrt(5.0f));
        for (int i = 0; i < sparksPerBurst; ++i) {
            float y = 1.0f - (i + 0.5f) / sparksPerBurst * 2.0f;
            float r = std::sqrt(1.0f - y * y);
            float a = golden * i;
            glm::vec3 dir(std::cos(a) * r, y, std::sin(a) * r);

            glm::vec3 c = (twoTone && (i % 2)) ? second : col;
            spawnSpark(scene, pos, vel * 0.2f + dir * speed * rng.range(0.85f, 1.1f), c, 0, rng.range(1.8f, 2.6f), sparkSize);
        }
    }

    // spark
    void updateSpark(float dt) {
        vel.y -= 5.5f * dt;
        vel *= std::exp(-1.4f * dt);   // air drag
        pos += vel * dt;

        float t = glm::clamp(age / lifetime, 0.0f, 1.0f);
        float fade = 1.0f - t * t * t;

        // white hot at birth, cools to its colour
        glm::vec3 c = glm::mix(glm::vec3(1.0f), col, glm::clamp(t * 3.0f, 0.0f, 1.0f));

        TransformComponent tr = entity.getTransform();
        tr.setPosition(pos);
        tr.setScale(glm::vec3(baseScale * (0.35f + 0.65f * fade)));

        if (Material* m = Demo::materialOf(entity)) {
            m->albedo = c * 0.5f;
            m->emission = c * 2.5f * fade * brightness;
        }

        if (age < lifetime) return;

        if (generation == 0 && rng.unit() < crackleChance) {
            Scene& scene = entity.getScene();
            for (int i = 0; i < 4; ++i) {
                glm::vec3 d(rng.range(-1, 1), rng.range(-1, 1), rng.range(-1, 1));
                spawnSpark(scene, pos, vel + d * 3.0f, glm::vec3(1.0f, 0.95f, 0.8f), 1, rng.range(0.3f, 0.5f), sparkSize * 0.45f);
            }
        }
        die();
    }

    void spawnSpark(Scene& scene, const glm::vec3& at, const glm::vec3& v, const glm::vec3& c,
        int gen, float life, float size) {
        Entity s = Demo::spawnPrimitive(scene, "Spark", Demo::PrimSphere, at, glm::vec3(size), c * 0.5f);
        if (Material* m = Demo::materialOf(s)) { m->emission = c * 2.5f * brightness; m->specularReflectance = 0.0f; }
        inheritSettings(s.addComponent<Firework>(Spark, gen, v, c, life));
    }

    void die() {
        dead = true;
        entity.getScene().removeEntity(entity);
    }

    int generation = 0;
    glm::vec3 vel = glm::vec3(0.0f);
    glm::vec3 col = glm::vec3(1.0f);
    glm::vec3 pos = glm::vec3(0.0f);
    float lifetime = 2.0f;
    float age = 0.0f;
    float burstAge = 0.0f;
    float timer = 0.0f;
    float baseScale = 1.0f;
    int salvo = 0;
    bool burst = false;
    bool dead = false;
    Demo::Rng rng;
END_SCRIPT(Firework)
