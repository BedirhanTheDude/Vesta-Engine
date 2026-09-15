#pragma once

#include <scripting/Script.h>

#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>

SCRIPT(Spinner)
public:
    SERIALIZE(float, speed, 1.0f);

    void onUpdate(float dt) override {
        Entity* entity = getEntity();
        if (!entity) return;

        glm::vec3 rotation = entity->transform.getEulerRotation();
        rotation.y += degreesPerSecond * dt * speed;
        entity->transform.setRotation(rotation);
    }

private:
    float degreesPerSecond = 90.0f;
END_SCRIPT(Spinner);
