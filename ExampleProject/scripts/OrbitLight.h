#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <scene/components/PointLightComponent.h>

// Point light that orbits a centre point on a wobbling ring and cycles its hue
BEGIN_SCRIPT(OrbitLight)
public:
    SERIALIZE(glm::vec3, center, glm::vec3(0.0f, 2.0f, 0.0f));
    SERIALIZE(float, radius, 4.0f);
    SERIALIZE(float, angularSpeed, 0.8f);
    SERIALIZE(float, phase, 0.0f);
    SERIALIZE(float, bobHeight, 1.0f);
    SERIALIZE(float, hueSpeed, 0.08f);
    SERIALIZE(float, hueOffset, 0.0f);
    SERIALIZE(glm::vec3, baseColor, glm::vec3(1.0f, 0.6f, 0.2f));
    SERIALIZE(float, intensity, 1.4f);

    void onStart() override {
        time = phase;
    }

    void onUpdate(float dt) override {
        time += dt * angularSpeed;

        glm::vec3 pos = center + glm::vec3(std::cos(time) * radius,
                                           std::sin(time * 2.3f) * bobHeight,
                                           std::sin(time) * radius);
        entity.getTransform().setPosition(pos);

        glm::vec3 color = baseColor;
        if (hueSpeed != 0.0f) {
            hue += dt * hueSpeed;
            color = Demo::hsv(hue + hueOffset, 0.85f, 1.0f);
        }

        if (auto light = entity.getComponent<PointLightComponent>()) {
            light->setColor(color);
            light->setDiffuseStrength(intensity);
            light->setSpecularStrength(intensity);
        }

        if (Material* bulb = Demo::materialOf(entity)) {
            bulb->albedo = color;
            bulb->emission = color * 1.5f;
        }
    }

private:
    float time = 0.0f;
    float hue = 0.0f;
END_SCRIPT(OrbitLight)
