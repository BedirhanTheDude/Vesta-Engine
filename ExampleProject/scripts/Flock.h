#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <core/Input.h>

#include <vector>

// Boids: sort of flies that run away from a predator
BEGIN_SCRIPT(Flock)
public:
    SERIALIZE(int, count, 60);
    SERIALIZE(float, boundsRadius, 9.0f);
    SERIALIZE(float, maxSpeed, 6.0f);
    SERIALIZE(float, neighbourRadius, 2.5f);
    SERIALIZE(float, separationRadius, 0.9f);
    SERIALIZE(float, separationWeight, 1.8f);
    SERIALIZE(float, alignmentWeight, 1.0f);
    SERIALIZE(float, cohesionWeight, 0.8f);
    SERIALIZE(float, fleeWeight, 6.0f);
    SERIALIZE(bool, predator, true);

    void onStart() override {
        Scene& scene = entity.getScene();
        Demo::Rng rng(4242u + entity.getID());

        boids.reserve(count);
        pos.reserve(count);
        vel.reserve(count);

        for (int i = 0; i < count; ++i) {
            glm::vec3 p(rng.range(-1, 1), rng.range(-0.5f, 0.5f), rng.range(-1, 1));
            p *= boundsRadius * 0.6f;
            glm::vec3 v = glm::normalize(glm::vec3(rng.range(-1, 1), rng.range(-0.2f, 0.2f), rng.range(-1, 1))) * maxSpeed * 0.5f;

            Entity b = Demo::spawnPrimitive(scene, "Boid", Demo::PrimCube, p, glm::vec3(0.12f, 0.12f, 0.45f),
                Demo::hsv(0.5f + 0.15f * rng.unit(), 0.6f, 1.0f), entity);
            if (Material* m = Demo::materialOf(b)) { m->specularReflectance = 0.9f; m->shininess = 96.0f; }

            boids.push_back(b);
            pos.push_back(p);
            vel.push_back(v);
        }

        if (predator) {
            hunter = Demo::spawnPrimitive(scene, "Predator", Demo::PrimSphere, glm::vec3(0.0f), glm::vec3(0.6f),
                glm::vec3(1.0f, 0.15f, 0.1f), entity);
            if (Material* m = Demo::materialOf(hunter)) m->emission = glm::vec3(1.5f, 0.2f, 0.05f);
            hunter.addComponent<PointLightComponent>(0.0f, 1.2f, 0.6f, 1.0f, 0.35f, 0.44f)
                .setColor(glm::vec3(1.0f, 0.25f, 0.1f));
        }
    }

    void onUpdate(float dt) override {
        time += dt;
        dt = (std::min)(dt, 1.0f / 30.0f);

        // predator wanders on a lissajous path, B makes it dive to the flock's centre of mass
        glm::vec3 predPos(0.0f);
        if (predator && hunter.isAlive()) {
            if (Input::isKeyPressed(Key::B)) diveTimer = 2.0f;

            glm::vec3 wander(std::sin(time * 0.37f) * boundsRadius * 0.8f,
                             std::sin(time * 0.71f) * boundsRadius * 0.3f,
                             std::cos(time * 0.53f) * boundsRadius * 0.8f);
            glm::vec3 centre(0.0f);
            for (const glm::vec3& p : pos) centre += p;
            if (!pos.empty()) centre /= float(pos.size());

            diveTimer = (std::max)(0.0f, diveTimer - dt);
            float dive = glm::smoothstep(0.0f, 0.5f, diveTimer);
            predPos = glm::mix(wander, centre, dive);
            hunter.getTransform().setPosition(predPos);
        }

        const float nr2 = neighbourRadius * neighbourRadius;
        const float sr2 = separationRadius * separationRadius;
        const size_t n = boids.size();

        for (size_t i = 0; i < n; ++i) {
            glm::vec3 sep(0.0f), ali(0.0f), coh(0.0f);
            int neighbours = 0;

            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;
                glm::vec3 d = pos[j] - pos[i];
                float d2 = glm::dot(d, d);
                if (d2 > nr2) continue;

                ali += vel[j];
                coh += pos[j];
                ++neighbours;
                if (d2 < sr2 && d2 > 1e-6f) sep -= d / d2;
            }

            glm::vec3 accel(0.0f);
            if (neighbours > 0) {
                ali = ali / float(neighbours) - vel[i];
                coh = coh / float(neighbours) - pos[i];
                accel += ali * alignmentWeight + coh * cohesionWeight;
            }
            accel += sep * separationWeight;

            // soft spherical boundary
            glm::vec3 p = pos[i] * glm::vec3(1.0f, 2.0f, 1.0f);
            float dist = glm::length(p);
            if (dist > boundsRadius) accel -= glm::normalize(p) * (dist - boundsRadius) * 2.0f;

            if (predator) {
                glm::vec3 away = pos[i] - predPos;
                float ad2 = glm::dot(away, away);
                if (ad2 < 16.0f && ad2 > 1e-6f) accel += glm::normalize(away) * fleeWeight * (1.0f - ad2 / 16.0f) * 4.0f;
            }

            vel[i] += accel * dt;
            float speed = glm::length(vel[i]);
            float minSpeed = maxSpeed * 0.35f;
            if (speed > maxSpeed) vel[i] *= maxSpeed / speed;
            else if (speed < minSpeed && speed > 1e-4f) vel[i] *= minSpeed / speed;
        }

        for (size_t i = 0; i < n; ++i) {
            pos[i] += vel[i] * dt;
            if (!boids[i].isAlive()) continue;

            TransformComponent t = boids[i].getTransform();
            t.setPosition(pos[i]);
            if (glm::dot(vel[i], vel[i]) > 1e-6f)
                t.setRotation(Demo::lookRotation(vel[i]));

            // boids flush hot when panicking
            if (Material* m = Demo::materialOf(boids[i])) {
                float panic = glm::clamp((glm::length(vel[i]) / maxSpeed - 0.6f) * 2.5f, 0.0f, 1.0f);
                m->emission = glm::vec3(1.0f, 0.4f, 0.1f) * panic * 0.8f;
            }
        }
    }

private:
    std::vector<Entity> boids;
    std::vector<glm::vec3> pos;
    std::vector<glm::vec3> vel;
    Entity hunter;
    float time = 0.0f;
    float diveTimer = 0.0f;
END_SCRIPT(Flock)
