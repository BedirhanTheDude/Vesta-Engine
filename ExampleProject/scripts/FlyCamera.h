#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <core/Input.h>

// Camera controller for the demo, two modes (C toggles):
//   Fly    : hold Right mouse to look, WASD move, Q/E down/up, Shift = fast
//   Orbit  : cinematic slow orbit around orbitCenter with a gentle height sway
BEGIN_SCRIPT(FlyCamera)
public:
    SERIALIZE(float, moveSpeed, 8.0f);
    SERIALIZE(float, fastMultiplier, 3.0f);
    SERIALIZE(float, lookSensitivity, 0.15f);
    SERIALIZE(bool, startInOrbit, true);
    SERIALIZE(glm::vec3, orbitCenter, glm::vec3(0.0f, 2.0f, 0.0f));
    SERIALIZE(float, orbitRadius, 30.0f);
    SERIALIZE(float, orbitHeight, 11.0f);
    SERIALIZE(float, orbitSpeed, 0.12f);

    void onStart() override {
        orbit = startInOrbit;

        glm::vec3 f = entity.getTransform().forward();
        yaw = glm::degrees(std::atan2(-f.x, -f.z));
        pitch = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));

        glm::vec3 p = entity.getTransform().getPosition() - orbitCenter;
        orbitAngle = std::atan2(p.z, p.x);
    }

    void onUpdate(float dt) override {
        if (Input::isKeyPressed(Key::C)) {
            orbit = !orbit;
            if (!orbit) {
                
                glm::vec3 f = entity.getTransform().forward();
                yaw = glm::degrees(std::atan2(-f.x, -f.z));
                pitch = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));
            }
            else {
                glm::vec3 p = entity.getTransform().getPosition() - orbitCenter;
                orbitAngle = std::atan2(p.z, p.x);
            }
        }

        TransformComponent t = entity.getTransform();

        if (orbit) {
            time += dt;
            orbitAngle += orbitSpeed * dt;
            float h = orbitHeight + std::sin(time * 0.21f) * 4.0f;
            float r = orbitRadius + std::sin(time * 0.13f) * 6.0f;
            glm::vec3 p = orbitCenter + glm::vec3(std::cos(orbitAngle) * r, h, std::sin(orbitAngle) * r);
            t.setPosition(p);
            t.setRotation(Demo::lookRotation(orbitCenter - p));
            return;
        }

        if (Input::isMouseDown(MouseButton::Right)) {
            glm::vec2 d = Input::getMouseDelta();
            yaw -= d.x * lookSensitivity;
            pitch = glm::clamp(pitch - d.y * lookSensitivity, -89.0f, 89.0f);
        }
        t.setRotation(Demo::yawPitch(yaw, pitch));

        glm::vec3 move(0.0f);
        if (Input::isKeyDown(Key::W)) move += t.forward();
        if (Input::isKeyDown(Key::S)) move -= t.forward();
        if (Input::isKeyDown(Key::D)) move += t.right();
        if (Input::isKeyDown(Key::A)) move -= t.right();
        if (Input::isKeyDown(Key::E)) move += glm::vec3(0, 1, 0);
        if (Input::isKeyDown(Key::Q)) move -= glm::vec3(0, 1, 0);

        if (glm::dot(move, move) > 0.0f) {
            float speed = moveSpeed * (Input::isKeyDown(Key::LeftShift) ? fastMultiplier : 1.0f);
            t.translate(glm::normalize(move) * speed * dt);
        }
    }

private:
    bool orbit = true;
    float yaw = 0.0f;
    float pitch = 0.0f;
    float orbitAngle = 0.0f;
    float time = 0.0f;
END_SCRIPT(FlyCamera)
