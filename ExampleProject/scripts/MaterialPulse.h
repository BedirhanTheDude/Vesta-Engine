#pragma once

#include <scripting/Script.h>
#include "Common.h"

// Animates every material on the entity
// Modes:
//   0 = emission pulse (glowing heartbeat)
//   1 = rainbow albedo (+ a soft glow in the same colour)
//   2 = alpha breathing (makes the material transparent)
//   3 = shininess sweep (watch the specular highlight tighten and spread)
BEGIN_SCRIPT(MaterialPulse)
public:
    SERIALIZE(int, mode, 0);
    SERIALIZE(float, speed, 1.0f);
    SERIALIZE(float, phase, 0.0f);
    SERIALIZE(glm::vec3, glowColor, glm::vec3(0.2f, 0.8f, 1.0f));
    SERIALIZE(bool, castLight, true);
    SERIALIZE(float, lightIntensity, 4.0f);

    void onStart() override {
        time = phase;

        bool glows = (mode == 0 || mode == 1);
        if (castLight && glows && !entity.hasComponent<PointLightComponent>()) {
            entity.addComponent<PointLightComponent>(0.0f, 0.0f, 0.0f, 1.0f, 0.35f, 0.44f);
        }
    }

    void onUpdate(float dt) override {
        time += dt * speed;

        auto mc = entity.getComponent<MaterialComponent>();
        if (!mc) return;

        // heartbeat curve: two quick bumps then rest
        float beat = std::fmod(time, 1.6f);
        float pulse = std::exp(-std::pow((beat - 0.15f) * 14.0f, 2.0f)) +
                      0.6f * std::exp(-std::pow((beat - 0.45f) * 14.0f, 2.0f));
        float wave = 0.5f + 0.5f * std::sin(time * 2.0f);
        glm::vec3 rainbow = Demo::hsv(time * 0.15f, 0.8f, 1.0f);

        // the light follows exactly what the material emits
        if (auto light = entity.getComponent<PointLightComponent>()) {
            glm::vec3 color = (mode == 1) ? rainbow : glowColor;
            float k = (mode == 1) ? 0.6f : (0.15f + pulse * 2.0f) / 2.15f;
            light->setColor(color);
            light->setDiffuseStrength(lightIntensity * k);
            light->setSpecularStrength(lightIntensity * k * 0.6f);
        }

        const size_t count = mc->getMaterialCount();
        for (size_t i = 0; i < count; ++i) {
            Material* m = mc->getMaterial(int(i));
            if (!m) continue;

            switch (mode) {
            case 0:
                m->emission = glowColor * (0.15f + pulse * 2.0f);
                break;
            case 1:
                m->albedo = Demo::hsv(time * 0.15f + float(i) * 0.1f, 0.8f, 1.0f);
                m->emission = m->albedo * 0.35f;
                break;
            case 2:
                m->transparent = true;
                m->alpha = 0.15f + 0.8f * wave;
                break;
            case 3:
                m->specularReflectance = 1.0f;
                m->shininess = 2.0f + std::pow(wave, 3.0f) * 254.0f;
                break;
            default: break;
            }
        }
    }

private:
    float time = 0.0f;
END_SCRIPT(MaterialPulse)
