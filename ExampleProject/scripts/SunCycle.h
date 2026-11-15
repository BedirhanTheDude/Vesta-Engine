#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include <scene/components/DirectionalLightComponent.h>
#include <core/Input.h>

// Day/night cycle for a directional light
// N: toggle pause  [ / ]: slower / faster
BEGIN_SCRIPT(SunCycle)
public:
    SERIALIZE(float, dayLengthSeconds, 40.0f);
    SERIALIZE(float, startTimeOfDay, 0.3f);
    SERIALIZE(float, azimuth, -35.0f);
    SERIALIZE(bool, paused, false);

    void onStart() override {
        timeOfDay = startTimeOfDay;
    }

    void onUpdate(float dt) override {
        if (Input::isKeyPressed(Key::N)) paused = !paused;
        if (Input::isKeyPressed(Key::LeftBracket))  dayLengthSeconds *= 1.5f;
        if (Input::isKeyPressed(Key::RightBracket)) dayLengthSeconds /= 1.5f;

        if (!paused && dayLengthSeconds > 0.01f) {
            timeOfDay += dt / dayLengthSeconds;
            timeOfDay -= std::floor(timeOfDay);
        }

        // elevation: -90 at midnight, +90 at noon
        float angle = (timeOfDay - 0.25f) * glm::two_pi<float>();
        float elevation = std::sin(angle);               // -1..1
        float elevationDeg = glm::degrees(std::asin(elevation));

        bool isDay = elevation > -0.05f;
        // at night the "sun" light becomes the moon on the opposite side of the sky
        float lightPitch = isDay ? -(std::max)(elevationDeg, 4.0f) : -(std::max)(-elevationDeg, 15.0f);
        float lightYaw = azimuth + (isDay ? glm::degrees(angle) * 0.5f : 180.0f + glm::degrees(angle) * 0.5f);

        entity.getTransform().setRotation(Demo::yawPitch(lightYaw, lightPitch));

        auto sun = entity.getComponent<DirectionalLightComponent>();
        if (!sun) return;

        glm::vec3 noon(1.0f, 0.97f, 0.9f);
        glm::vec3 horizon(1.0f, 0.45f, 0.18f);
        glm::vec3 moon(0.35f, 0.45f, 0.8f);

        if (isDay) {
            float h = glm::clamp(elevation * 2.5f, 0.0f, 1.0f);   // 0 at horizon -> 1 high in the sky
            sun->setColor(glm::mix(horizon, noon, h));
            sun->setDiffuseStrength(0.25f + 0.75f * h);
            sun->setSpecularStrength(0.2f + 0.4f * h);
            sun->setAmbientStrength(0.08f + 0.14f * h);
        }
        else {
            sun->setColor(moon);
            sun->setDiffuseStrength(0.18f);
            sun->setSpecularStrength(0.1f);
            sun->setAmbientStrength(0.05f);
        }
    }

private:
    float timeOfDay = 0.3f;
END_SCRIPT(SunCycle)
