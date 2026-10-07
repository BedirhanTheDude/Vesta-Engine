#pragma once

#include <scripting/Script.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

#include <glm/glm.hpp>

#include <cmath>

BEGIN_SCRIPT(Spinner)
public:
    SERIALIZE(float, speed, 1.0f);

    void onUpdate(float dt) override {
        //timeElapsed += dt * 2.0f;

        TransformComponent transform = entity.getTransform();

        transform.rotateAroundLocalAxis(glm::vec3(-1, 0, 0), degreesPerSecond * dt * speed);
        //transform.setScale(glm::vec3(1, 1, 1) * (2 + std::sin(timeElapsed)));

    }

private:
    float degreesPerSecond = 90.0f;
    //float timeElapsed = 0.0f;
END_SCRIPT(Spinner)
