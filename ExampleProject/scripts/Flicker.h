#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <scene/components/PointLightComponent.h>

// Fire / candle flicker
BEGIN_SCRIPT(Flicker)
public:
    SERIALIZE(glm::vec3, warmColor, glm::vec3(1.0f, 0.55f, 0.15f));
    SERIALIZE(glm::vec3, hotColor, glm::vec3(1.0f, 0.85f, 0.5f));
    SERIALIZE(float, baseIntensity, 1.3f);
    SERIALIZE(float, flickerAmount, 0.7f);
    SERIALIZE(float, flickerSpeed, 9.0f);
    SERIALIZE(float, jitter, 0.04f);

    void onStart() override {
        anchor = entity.getTransform().getPosition();
        seed = float(entity.getID()) * 17.13f;
    }

    void onUpdate(float dt) override {
        time += dt * flickerSpeed;

        float n = Demo::noise1(time + seed) * 0.65f + Demo::noise1(time * 2.7f + seed * 3.1f) * 0.35f;
        float k = glm::clamp(1.0f - flickerAmount + n * flickerAmount * 2.0f, 0.0f, 2.0f);
        glm::vec3 color = glm::mix(warmColor, hotColor, n);

        if (auto light = entity.getComponent<PointLightComponent>()) {
            light->setColor(color);
            light->setDiffuseStrength(baseIntensity * k);
            light->setSpecularStrength(baseIntensity * k * 0.5f);
        }

        if (Material* flame = Demo::materialOf(entity)) {
            flame->albedo = color;
            flame->emission = color * (0.8f + k);
        }

        glm::vec3 offset(Demo::noise1(time * 0.7f + 11.0f) - 0.5f,
                         Demo::noise1(time * 0.9f + 23.0f) - 0.5f,
                         Demo::noise1(time * 0.8f + 37.0f) - 0.5f);
        entity.getTransform().setPosition(anchor + offset * jitter * 2.0f);
    }

private:
    glm::vec3 anchor = glm::vec3(0.0f);
    float time = 0.0f;
    float seed = 0.0f;
END_SCRIPT(Flicker)
