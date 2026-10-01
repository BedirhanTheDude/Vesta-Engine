#pragma once

#include <scripting/Script.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

SCRIPT(Spinner)
public:
    SERIALIZE(float, speed, 1.0f);

    void onUpdate(float dt) override {
        TransformComponent transform = entity.getTransform();

        glm::vec3 rotation = transform.getEulerRotation();
        rotation.y += degreesPerSecond * dt * speed;
        transform.setRotation(rotation);
    }

private:
    float degreesPerSecond = 90.0f;
END_SCRIPT(Spinner);
