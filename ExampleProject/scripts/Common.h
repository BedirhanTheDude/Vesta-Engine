#pragma once

// Shared helpers for the demo scripts. Not a script itself (no script macros in here),
// so the script scanner skips it; the demo scripts include it by relative path.
//
// NOTE: ScriptsDLL.cpp includes <Windows.h> before any script, so the min/max/near/far
// macros are live in every script. Use (std::min)(...) and avoid naming anything near/far.

#include <scene/Scene.h>
#include <scene/Entity.h>
#include <scene/components/TransformComponent.h>
#include <scene/components/MeshComponent.h>
#include <scene/components/MaterialComponent.h>
#include <scene/components/RigidBodyComponent.h>
#include <scene/components/PointLightComponent.h>
#include <renderer/Material.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>

#include <cmath>
#include <cstdint>
#include <string>

namespace Demo {

    // indices understood by MeshComponent::setPrimitive
    enum Primitive : unsigned int { PrimNone = 0, PrimCube = 1, PrimSphere = 2, PrimQuad = 3 };

    // h, s, v in [0, 1]
    inline glm::vec3 hsv(float h, float s, float v) {
        h = h - std::floor(h);
        float c = v * s;
        float hp = h * 6.0f;
        float x = c * (1.0f - std::fabs(std::fmod(hp, 2.0f) - 1.0f));
        glm::vec3 rgb(0.0f);
        if      (hp < 1.0f) rgb = { c, x, 0 };
        else if (hp < 2.0f) rgb = { x, c, 0 };
        else if (hp < 3.0f) rgb = { 0, c, x };
        else if (hp < 4.0f) rgb = { 0, x, c };
        else if (hp < 5.0f) rgb = { x, 0, c };
        else                rgb = { c, 0, x };
        return rgb + glm::vec3(v - c);
    }

    // tiny deterministic RNG, trivially movable (script pools move scripts around)
    // engine needs a core math libray for these kinds of things
    struct Rng {
        uint32_t state = 0x9E3779B9u;
        explicit Rng(uint32_t seed = 0x9E3779B9u) : state(seed ? seed : 1u) {}
        uint32_t next() { state ^= state << 13; state ^= state >> 17; state ^= state << 5; return state; }
        float unit() { return (next() & 0xFFFFFF) / float(0x1000000); }          // [0, 1)
        float range(float a, float b) { return a + (b - a) * unit(); }
    };

    inline float hash1(float n) { float s = std::sin(n) * 43758.5453f; return s - std::floor(s); }
    inline float noise1(float x) {
        float i = std::floor(x), f = x - i;
        float u = f * f * (3.0f - 2.0f * f);
        return hash1(i) * (1.0f - u) + hash1(i + 1.0f) * u;
    }

    inline Entity spawnPrimitive(Scene& scene, const std::string& name, unsigned int primitive,
        const glm::vec3& position, const glm::vec3& scale, const glm::vec3& albedo,
        const Entity& parent = Entity()) {

        Entity e = parent.isValid() ? scene.createEntity(name, parent) : scene.createEntity(name);

        TransformComponent t = e.getTransform();
        t.setPosition(position);
        t.setScale(scale);

        MeshComponent mesh = e.addComponent<MeshComponent>();
        mesh.setPrimitive(primitive);

        MaterialComponent mats = e.addComponent<MaterialComponent>();
        if (Material* m = mats.getMaterial(0))
            m->albedo = albedo;

        return e;
    }

    inline Entity spawnBody(Scene& scene, const std::string& name, unsigned int primitive,
        const glm::vec3& position, const glm::vec3& scale, const glm::vec3& albedo,
        RigidBodyType type, float restitution = 0.2f, float friction = 0.6f) {

        Entity e = spawnPrimitive(scene, name, primitive, position, scale, albedo);

        RigidBodyComponent rb = e.addComponent<RigidBodyComponent>(type);
        rb.setRestitution(restitution);
        rb.setStaticFriction(friction);
        rb.setDynamicFriction(friction * 0.8f);

        return e;
    }

    inline Material* materialOf(const Entity& e, int index = 0) {
        auto mc = e.getComponent<MaterialComponent>();
        return mc ? mc->getMaterial(index) : nullptr;
    }

    inline Entity spawnGroup(Scene& scene, const std::string& name, const glm::vec3& position = glm::vec3(0.0f)) {
        Entity e = scene.createEntity(name);
        e.getTransform().setPosition(position);
        return e;
    }

    inline glm::quat yawPitch(float yawDeg, float pitchDeg) {
        return glm::angleAxis(glm::radians(yawDeg), glm::vec3(0, 1, 0)) *
               glm::angleAxis(glm::radians(pitchDeg), glm::vec3(1, 0, 0));
    }

    inline glm::quat lookRotation(const glm::vec3& dir) {
        glm::vec3 f = glm::normalize(dir);
        float yaw = glm::degrees(std::atan2(-f.x, -f.z));
        float pitch = glm::degrees(std::asin(glm::clamp(f.y, -1.0f, 1.0f)));
        return yawPitch(yaw, pitch);
    }
}
