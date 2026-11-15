#pragma once

#include <scripting/Script.h>
#include "Common.h"
#include "Lifetime.h"
#include "Explosive.h"
#include "Contagion.h"

#include <core/Input.h>

#include <vector>

// Physics collision showcase, built around this entity's position
// Controls:
//   F / Left mouse : shoot a heavy ball from the active camera
//   X              : shoot an explosive charge
//   I              : infect a random body in the arena
//   R              : rebuild the tower + lineup
//   G              : toggle cube rain
BEGIN_SCRIPT(PhysicsPlayground)
public:
    SERIALIZE(int, towerLayers, 8);
    SERIALIZE(int, towerWidth, 4);
    SERIALIZE(float, projectileSpeed, 28.0f);
    SERIALIZE(bool, rain, true);
    SERIALIZE(float, rainInterval, 0.35f);
    SERIALIZE(float, debrisLifetime, 25.0f);
    SERIALIZE(bool, sweeper, true);
    SERIALIZE(float, sweeperSpeed, 0.6f);

    void onStart() override {
        origin = entity.getTransform().getWorldPosition();
        Scene& scene = entity.getScene();

        ramp = Demo::spawnPrimitive(scene, "Physics Ramp", Demo::PrimCube,
            origin + glm::vec3(-7.0f, 2.2f, -6.0f), glm::vec3(3.0f, 0.3f, 10.0f),
            glm::vec3(0.35f, 0.35f, 0.4f));
        ramp.getTransform().setRotation(glm::vec3(20.0f, 0.0f, 0.0f));
        {
            RigidBodyComponent rb = ramp.addComponent<RigidBodyComponent>(Static);
            rb.setRestitution(0.1f);
            rb.setStaticFriction(0.3f);
            rb.setDynamicFriction(0.25f);
        }

        if (sweeper) {
            sweeperArm = Demo::spawnBody(scene, "Sweeper Arm", Demo::PrimCube,
                origin + glm::vec3(0.0f, 0.6f, 0.0f), glm::vec3(9.0f, 0.8f, 0.6f),
                glm::vec3(0.9f, 0.75f, 0.1f), Kinematic);
            if (Material* m = Demo::materialOf(sweeperArm)) {
                m->emission = glm::vec3(0.25f, 0.18f, 0.0f);
                m->specularReflectance = 0.8f;
            }
        }

        buildSet();
    }

    void onUpdate(float dt) override {
        time += dt;
        Scene& scene = entity.getScene();

        if (Input::isKeyPressed(Key::F) || Input::isMousePressed(MouseButton::Left))
            shoot(scene, false);
        if (Input::isKeyPressed(Key::X))
            shoot(scene, true);

        if (Input::isKeyPressed(Key::R)) {
            clearSet(scene);
            buildSet();
        }

        if (Input::isKeyPressed(Key::I)) infectRandomBody(scene);

        if (Input::isKeyPressed(Key::G)) rain = !rain;

        if (sweeper && sweeperArm.isAlive()) {
            float a = time * sweeperSpeed;
            TransformComponent st = sweeperArm.getTransform();
            st.setPosition(origin + glm::vec3(std::cos(a) * 7.0f, 0.6f, std::sin(a) * 7.0f));
            st.setRotation(glm::angleAxis(-a, glm::vec3(0, 1, 0)));
        }

        // marbles down the ramp
        marbleTimer += dt;
        if (marbleTimer > 1.2f) {
            marbleTimer = 0.0f;
            glm::vec3 top = origin + glm::vec3(-7.0f + rng.range(-1.0f, 1.0f), 5.5f, -10.0f);
            Entity m = Demo::spawnBody(scene, "Marble", Demo::PrimSphere, top, glm::vec3(0.5f),
                Demo::hsv(rng.unit(), 0.6f, 1.0f), Dynamic, 0.4f, 0.2f);
            if (Material* mat = Demo::materialOf(m)) { mat->specularReflectance = 1.0f; mat->shininess = 128.0f; }
            m.addComponent<Lifetime>(debrisLifetime);
        }

        if (rain) {
            rainTimer += dt;
            if (rainTimer > rainInterval) {
                rainTimer = 0.0f;
                glm::vec3 p = origin + glm::vec3(rng.range(-6.0f, 6.0f), 14.0f, rng.range(-6.0f, 6.0f));
                float s = rng.range(0.35f, 0.8f);
                bool sphere = rng.unit() < 0.35f;
                Entity e = Demo::spawnBody(scene, "Debris", sphere ? Demo::PrimSphere : Demo::PrimCube,
                    p, glm::vec3(s), Demo::hsv(rng.unit(), 0.55f, 0.95f), Dynamic, rng.range(0.05f, 0.6f));
                if (auto rb = e.getComponent<RigidBodyComponent>())
                    rb->setAngularVelocity(glm::vec3(rng.range(-4, 4), rng.range(-4, 4), rng.range(-4, 4)));
                e.addComponent<Lifetime>(debrisLifetime);
            }
        }
    }

private:
    void buildSet() {
        Scene& scene = entity.getScene();

        // brick tower
        const float brick = 1.0f;
        for (int y = 0; y < towerLayers; ++y) {
            glm::vec3 layerColor = (y % 2 == 0) ? glm::vec3(0.75f, 0.28f, 0.2f) : glm::vec3(0.85f, 0.8f, 0.7f);
            float shift = (y % 2 == 0) ? 0.0f : brick * 0.5f;
            for (int x = 0; x < towerWidth; ++x) {
                glm::vec3 p = origin + glm::vec3(4.0f + x * brick * 1.02f + shift, 0.5f + y * brick * 1.001f, 2.0f);
                set.push_back(Demo::spawnBody(scene, "Brick", Demo::PrimCube, p, glm::vec3(brick * 0.98f),
                    layerColor, Dynamic, 0.05f, 0.8f));
            }
        }

        // restitution lineup
        for (int i = 0; i < 5; ++i) {
            float r = i / 4.0f * 0.95f;
            glm::vec3 p = origin + glm::vec3(-3.0f + i * 1.5f, 9.0f, 6.0f);
            Entity ball = Demo::spawnBody(scene, "Bounce " + std::to_string(int(r * 100)) + "%", Demo::PrimSphere,
                p, glm::vec3(0.9f), glm::mix(glm::vec3(0.2f, 0.3f, 0.9f), glm::vec3(1.0f, 0.2f, 0.5f), r / 0.95f),
                Dynamic, r, 0.5f);
            if (Material* m = Demo::materialOf(ball)) m->emission = glm::vec3(r * 0.25f);
            set.push_back(ball);
        }

        // patient zero: this entity infects every other entity it touches
        if (towerLayers > 0 && towerWidth > 0) {
            size_t bricks = size_t(towerLayers) * size_t(towerWidth);
            Entity zero = set[size_t(rng.unit() * bricks) % bricks];
            zero.addComponent<Contagion>();
        }
    }

    void infectRandomBody(Scene& scene) {
        Entity pick;
        int seen = 0;
        for (const Entity& e : scene.getEntities()) {
            if (e.hasComponent<Contagion>()) continue;
            auto rb = e.getComponent<RigidBodyComponent>();
            if (!rb || rb->getType() != Dynamic) continue;

            glm::vec3 d = rb->getWorldPosition() - origin;
            if (std::fabs(d.x) > 13.0f || std::fabs(d.z) > 13.0f) continue; // inside the arena walls

            if (rng.unit() * float(++seen) < 1.0f) pick = e;
        }
        if (pick.isValid()) pick.addComponent<Contagion>();
    }

    void clearSet(Scene& scene) {
        for (const Entity& e : set)
            if (e.isAlive()) scene.removeEntity(e);
        set.clear();
    }

    void shoot(Scene& scene, bool explosive) {
        Entity cam = scene.getActiveCameraEntity();
        if (!cam.isValid()) return;

        TransformComponent ct = cam.getTransform();
        glm::vec3 dir = ct.forward();
        glm::vec3 start = ct.getWorldPosition() + dir * 1.5f;

        Entity ball = Demo::spawnBody(scene, explosive ? "Charge" : "Cannonball", explosive ? Demo::PrimCube : Demo::PrimSphere,
            start, glm::vec3(explosive ? 0.6f : 0.8f), explosive ? glm::vec3(0.6f, 0.1f, 0.05f) : glm::vec3(0.12f),
            Dynamic, 0.3f, 0.5f);
        if (Material* m = Demo::materialOf(ball)) {
            m->specularReflectance = 1.0f;
            m->shininess = 200.0f;
            m->emission = glm::vec3(0.6f, 0.15f, 0.0f);
        }
        if (auto rb = ball.getComponent<RigidBodyComponent>())
            rb->setLinearVelocity(dir * projectileSpeed);

        if (explosive) ball.addComponent<Explosive>();
        else ball.addComponent<Lifetime>(debrisLifetime);
    }

    glm::vec3 origin = glm::vec3(0.0f);
    Entity ramp;
    Entity sweeperArm;
    std::vector<Entity> set;
    Demo::Rng rng{ 1337u };
    float time = 0.0f;
    float rainTimer = 0.0f;
    float marbleTimer = 0.0f;
END_SCRIPT(PhysicsPlayground)
