#pragma once

#include <scripting/Script.h>
#include "Common.h"

// Self-destruct timer
BEGIN_SCRIPT(Lifetime)
public:
    SERIALIZE(float, lifetime, 20.0f);
    SERIALIZE(float, fadeTime, 1.5f);
    SERIALIZE(float, killBelowY, -20.0f);

    void init(float seconds) { lifetime = seconds; }

    void onUpdate(float dt) override {
        if (dead) return;
        age += dt;

        if (entity.getTransform().getWorldPosition().y < killBelowY || age >= lifetime) {
            dead = true;
            entity.getScene().removeEntity(entity);
            return;
        }

        float remaining = lifetime - age;
        if (remaining >= fadeTime || fadeTime <= 0.0f) return;

        float alpha = remaining / fadeTime;
        if (auto mc = entity.getComponent<MaterialComponent>()) {
            const size_t count = mc->getMaterialCount();
            for (size_t i = 0; i < count; ++i)
                if (Material* m = mc->getMaterial(int(i))) {
                    m->transparent = true;
                    m->alpha = alpha;
                }
        }
    }

private:
    float age = 0.0f;
    bool dead = false;
END_SCRIPT(Lifetime)
