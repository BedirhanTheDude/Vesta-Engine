#pragma once

#include <scripting/Script.h>
#include "Common.h"

#include "OrbitLight.h"
#include "Flicker.h"
#include "SunCycle.h"
#include "MaterialPulse.h"
#include "PhysicsPlayground.h"
#include "Turntable.h"
#include "Flock.h"
#include "FlyCamera.h"
#include "Firework.h"
#include "Lifetime.h"
#include "Explosive.h"
#include "Car.h"

#include <core/AssetManager.h>
#include <scene/components/DirectionalLightComponent.h>
#include <scene/components/CameraComponent.h>

// ONE-CLICK DEMO SCENE
// Make a new scene, add an empty entity, attach Director, press Play
//
// Controls: C camera mode | RMB+WASD/QE fly | F/LMB shoot | X explosive | I infect a body
//           R rebuild tower | G rain | B predator dive | L firework salvo | N pause sun | [ ] day speed
BEGIN_SCRIPT(Director)
public:
    SERIALIZE(bool, buildPhysics, true);
    SERIALIZE(bool, buildLighting, true);
    SERIALIZE(bool, buildMaterials, true);
    SERIALIZE(bool, buildShowcase, true);
    SERIALIZE(bool, buildFlock, true);
    SERIALIZE(bool, buildFireworks, true);
    SERIALIZE(bool, buildCar, true);
    SERIALIZE(bool, dayNightCycle, true);
    SERIALIZE(bool, flyCamera, true);
    SERIALIZE(std::string, showcaseModel, std::string("models/maxwell.obj"));
    SERIALIZE(float, showcaseScale, 1.0f);
    SERIALIZE(std::string, floorTexture, std::string("models/wood.jpg"));

    void onStart() override {
        Scene& scene = entity.getScene();

        buildGround(scene);
        setupSunAndCamera(scene);

        if (buildPhysics)   buildPhysicsZone(scene, glm::vec3(-22.0f, 0.0f, 0.0f));
        if (buildLighting)  buildLightPavilion(scene, glm::vec3(22.0f, 0.0f, 0.0f));
        if (buildMaterials) buildMaterialGallery(scene, glm::vec3(0.0f, 0.0f, -20.0f));
        if (buildShowcase)  buildShowcasePedestal(scene, glm::vec3(0.0f, 0.0f, 0.0f));
        if (buildFlock)     buildBoids(scene, glm::vec3(0.0f, 9.0f, 0.0f));
        if (buildFireworks) buildFireworkLauncher(scene, glm::vec3(0.0f, 0.0f, 14.0f));
        if (buildCar)       scene.createEntity("BMW").addComponent<Car>();
    }

private:
    // ground
    void buildGround(Scene& scene) {
        Demo::spawnBody(scene, "Ground", Demo::PrimCube, glm::vec3(0.0f, -0.5f, 0.0f),
            glm::vec3(90.0f, 1.0f, 70.0f), glm::vec3(0.32f, 0.33f, 0.36f), Static, 0.2f, 0.7f);

        // a slightly raised walkway tile pattern so the floor isn't one flat colour
        Entity tiles = Demo::spawnGroup(scene, "Floor Tiles");
        for (int x = -4; x <= 4; ++x)
            for (int z = -3; z <= 3; ++z) {
                if ((x + z) % 2 != 0) continue;
                Entity t = Demo::spawnPrimitive(scene, "Tile", Demo::PrimQuad,
                    glm::vec3(x * 10.0f, 0.01f, z * 10.0f), glm::vec3(10.0f, 1.0f, 10.0f),
                    glm::vec3(0.27f, 0.28f, 0.31f), tiles);
                if (Material* m = Demo::materialOf(t)) m->specularReflectance = 0.05f;
            }
    }

    void setupSunAndCamera(Scene& scene) {
        Entity sun = scene.findEntity("Directional Light");
        if (!sun.isValid()) {
            sun = scene.createEntity("Directional Light");
            sun.addComponent<DirectionalLightComponent>();
        }
        if (auto light = sun.getComponent<DirectionalLightComponent>()) {
            light->setShadowOrthoSize(80.0f);
            light->setShadowDistance(60.0f);
            light->setShadowFar(250.0f);
        }
        if (dayNightCycle && !sun.hasComponent<SunCycle>())
            sun.addComponent<SunCycle>();

        Entity cam = scene.getActiveCameraEntity();
        if (!cam.isValid()) {
            cam = scene.createEntity("Main Camera");
            cam.addComponent<CameraComponent>();
            scene.setActiveCamera(cam);
        }
        if (auto camera = cam.getComponent<CameraComponent>())
            camera->setFarPlane(300.0f);

        TransformComponent ct = cam.getTransform();
        ct.setPosition(glm::vec3(0.0f, 12.0f, 34.0f));
        ct.setRotation(Demo::lookRotation(glm::vec3(0.0f, 2.0f, -4.0f) - glm::vec3(0.0f, 12.0f, 34.0f)));

        if (flyCamera && !cam.hasComponent<FlyCamera>())
            cam.addComponent<FlyCamera>();
    }

    // physics
    void buildPhysicsZone(Scene& scene, const glm::vec3& at) {
        const glm::vec3 wallColor(0.45f, 0.42f, 0.4f);
        Demo::spawnBody(scene, "Arena Wall N", Demo::PrimCube, at + glm::vec3(0, 0.75f, -13), glm::vec3(26, 1.5f, 0.6f), wallColor, Static);
        Demo::spawnBody(scene, "Arena Wall S", Demo::PrimCube, at + glm::vec3(0, 0.75f, 13), glm::vec3(26, 1.5f, 0.6f), wallColor, Static);
        Demo::spawnBody(scene, "Arena Wall W", Demo::PrimCube, at + glm::vec3(-13, 0.75f, 0), glm::vec3(0.6f, 1.5f, 26), wallColor, Static);
        Demo::spawnBody(scene, "Arena Wall E", Demo::PrimCube, at + glm::vec3(13, 0.75f, 0), glm::vec3(0.6f, 1.5f, 26), wallColor, Static);

        Entity controller = Demo::spawnGroup(scene, "Physics Playground", at);
        controller.addComponent<PhysicsPlayground>();
    }

    // lighting
    void buildLightPavilion(Scene& scene, const glm::vec3& at) {
        Entity group = Demo::spawnGroup(scene, "Light Pavilion", at);
        const glm::vec3 stone(0.55f, 0.53f, 0.5f);

        Demo::spawnPrimitive(scene, "Roof",      Demo::PrimCube, glm::vec3(0, 7.0f, 0),    glm::vec3(16, 0.5f, 16), stone, group);
        Demo::spawnPrimitive(scene, "Back Wall", Demo::PrimCube, glm::vec3(0, 3.5f, -7.75f), glm::vec3(16, 7, 0.5f), stone, group);
        Demo::spawnPrimitive(scene, "Left Wall", Demo::PrimCube, glm::vec3(-7.75f, 3.5f, 0), glm::vec3(0.5f, 7, 16), stone, group);
        Demo::spawnPrimitive(scene, "Right Wall",Demo::PrimCube, glm::vec3(7.75f, 3.5f, 0),  glm::vec3(0.5f, 7, 16), stone, group);

        for (int i = 0; i < 4; ++i) {
            float a = i * glm::half_pi<float>() + glm::quarter_pi<float>();
            glm::vec3 p(std::cos(a) * 4.0f, 2.0f, std::sin(a) * 4.0f);
            Entity pillar = Demo::spawnPrimitive(scene, "Pillar", Demo::PrimCube, p, glm::vec3(0.8f, 4.0f, 0.8f),
                glm::vec3(0.9f), group);
            if (Material* m = Demo::materialOf(pillar)) { m->specularReflectance = 0.6f; m->shininess = 64.0f; }
        }
        Entity orb = Demo::spawnPrimitive(scene, "Centre Orb", Demo::PrimSphere, glm::vec3(0, 1.6f, 0), glm::vec3(2.4f),
            glm::vec3(0.95f), group);
        if (Material* m = Demo::materialOf(orb)) { m->specularReflectance = 1.0f; m->shininess = 128.0f; }

        // three orbiting RGB-ish lights
        for (int i = 0; i < 3; ++i) {
            Entity bulb = Demo::spawnPrimitive(scene, "Orbit Light " + std::to_string(i + 1), Demo::PrimSphere,
                glm::vec3(0, 2, 0), glm::vec3(0.25f), glm::vec3(1.0f), group);
            bulb.addComponent<PointLightComponent>(0.02f, 1.4f, 1.0f, 1.0f, 0.22f, 0.20f);

            OrbitLight& orbit = bulb.addComponent<OrbitLight>();
            
            orbit.center = glm::vec3(0.0f, 2.5f, 0.0f);
            orbit.radius = 3.0f + i * 0.9f;
            orbit.angularSpeed = 0.6f + i * 0.25f;
            orbit.phase = i * glm::two_pi<float>() / 3.0f;
            orbit.hueOffset = i / 3.0f;
            orbit.bobHeight = 0.6f + i * 0.3f;
        }

        // two wall torches with flickering fire light
        for (int side = -1; side <= 1; side += 2) {
            glm::vec3 p(side * 7.2f, 3.2f, -3.0f);
            Demo::spawnPrimitive(scene, "Torch Holder", Demo::PrimCube, p - glm::vec3(0, 0.45f, 0),
                glm::vec3(0.25f, 0.6f, 0.25f), glm::vec3(0.25f, 0.18f, 0.12f), group);
            Entity flame = Demo::spawnPrimitive(scene, "Torch Flame", Demo::PrimSphere, p, glm::vec3(0.22f, 0.32f, 0.22f),
                glm::vec3(1.0f, 0.6f, 0.2f), group);
            flame.addComponent<PointLightComponent>(0.03f, 1.3f, 0.6f, 1.0f, 0.35f, 0.44f);
            flame.addComponent<Flicker>();
        }
    }

    // materials
    void buildMaterialGallery(Scene& scene, const glm::vec3& at) {
        Entity group = Demo::spawnGroup(scene, "Material Gallery", at);

        const int cols = 6, rows = 4;
        const float spacing = 2.6f;
        for (int r = 0; r < rows; ++r)
            for (int c = 0; c < cols; ++c) {
                float shininess = 4.0f * std::pow(2.0f, float(c) * 6.0f / (cols - 1));
                float spec = float(r) / (rows - 1);
                glm::vec3 p((c - (cols - 1) * 0.5f) * spacing, 0.0f, (r - (rows - 1) * 0.5f) * spacing);

                Demo::spawnPrimitive(scene, "Pedestal", Demo::PrimCube, p + glm::vec3(0, 0.3f, 0), glm::vec3(1.2f, 0.6f, 1.2f),
                    glm::vec3(0.2f), group);
                Entity s = Demo::spawnPrimitive(scene, "Sphere s" + std::to_string(int(shininess)) + " r" + std::to_string(int(spec * 100)),
                    Demo::PrimSphere, p + glm::vec3(0, 1.3f, 0), glm::vec3(1.4f), Demo::hsv(0.58f + r * 0.08f, 0.55f, 0.85f), group);
                if (Material* m = Demo::materialOf(s)) {
                    m->shininess = shininess;
                    m->specularReflectance = spec;
                }
            }

        const float z = (rows * 0.5f + 0.9f) * spacing;
        struct Special { const char* name; unsigned int prim; int mode; glm::vec3 color; };
        const Special specials[] = {
            { "Heartbeat Glow",   Demo::PrimSphere, 0, glm::vec3(0.1f, 0.6f, 1.0f) },
            { "Rainbow",          Demo::PrimCube,   1, glm::vec3(1.0f) },
            { "Breathing Glass",  Demo::PrimSphere, 2, glm::vec3(0.6f, 0.9f, 1.0f) },
            { "Shininess Sweep",  Demo::PrimSphere, 3, glm::vec3(0.8f, 0.2f, 0.25f) },
        };
        for (int i = 0; i < 4; ++i) {
            glm::vec3 p((i - 2.0f) * 3.4f, 0.0f, z);
            Demo::spawnPrimitive(scene, "Pedestal", Demo::PrimCube, p + glm::vec3(0, 0.4f, 0), glm::vec3(1.4f, 0.8f, 1.4f),
                glm::vec3(0.12f), group);
            Entity e = Demo::spawnPrimitive(scene, specials[i].name, specials[i].prim, p + glm::vec3(0, 1.7f, 0),
                glm::vec3(1.5f), specials[i].color, group);
            MaterialPulse& pulse = e.addComponent<MaterialPulse>();
            pulse.mode = specials[i].mode;
            pulse.glowColor = specials[i].color;
            pulse.phase = i * 0.4f;
        }

        glm::vec3 crateP(2.0f * 3.4f, 0.0f, z);
        Entity crate = Demo::spawnPrimitive(scene, "Wooden Crate", Demo::PrimCube, crateP + glm::vec3(0, 0.8f, 0),
            glm::vec3(1.6f), glm::vec3(1.0f), group);
        if (Material* m = Demo::materialOf(crate)) {
            if (!floorTexture.empty()) m->diffuseTexture = AssetManager::getTexture(floorTexture);
            m->specularReflectance = 0.1f;
            m->shininess = 8.0f;
        }

        Entity glass = Demo::spawnPrimitive(scene, "Glass Pane", Demo::PrimCube, glm::vec3(0.0f, 2.0f, z + 2.4f),
            glm::vec3(10.0f, 4.0f, 0.1f), glm::vec3(0.7f, 0.85f, 1.0f), group);
        if (Material* m = Demo::materialOf(glass)) {
            m->transparent = true;
            m->alpha = 0.18f;
            m->specularReflectance = 1.0f;
            m->shininess = 256.0f;
        }
    }

    // showcase
    void buildShowcasePedestal(Scene& scene, const glm::vec3& at) {
        Entity group = Demo::spawnGroup(scene, "Showcase", at);

        Entity base = Demo::spawnPrimitive(scene, "Showcase Base", Demo::PrimCube, glm::vec3(0, 0.4f, 0),
            glm::vec3(4.0f, 0.8f, 4.0f), glm::vec3(0.12f, 0.12f, 0.14f), group);
        if (Material* m = Demo::materialOf(base)) { m->specularReflectance = 0.9f; m->shininess = 200.0f; }

        Entity ring = Demo::spawnPrimitive(scene, "Showcase Ring", Demo::PrimCube, glm::vec3(0, 0.82f, 0),
            glm::vec3(3.6f, 0.05f, 3.6f), glm::vec3(0.1f, 0.6f, 1.0f), group);
        MaterialPulse& ringPulse = ring.addComponent<MaterialPulse>();
        ringPulse.mode = 0;
        ringPulse.speed = 0.6f;

        Entity model = Demo::spawnPrimitive(scene, "Showcase Model", Demo::PrimSphere, glm::vec3(0, 2.4f, 0),
            glm::vec3(1.0f), glm::vec3(0.9f), group);
        Turntable& turntable = model.addComponent<Turntable>();
        turntable.modelPath = showcaseModel;
        turntable.modelScale = showcaseScale;

        for (int i = 0; i < 2; ++i) {
            Entity key = Demo::spawnPrimitive(scene, "Showcase Key Light", Demo::PrimSphere, glm::vec3(0, 3, 0),
                glm::vec3(0.15f), glm::vec3(1.0f), group);
            key.addComponent<PointLightComponent>(0.0f, 1.0f, 1.0f, 1.0f, 0.35f, 0.44f);
            OrbitLight& o = key.addComponent<OrbitLight>();
            o.center = glm::vec3(0, 3.0f, 0);
            o.radius = 3.2f;
            o.angularSpeed = 0.4f;
            o.phase = i * glm::pi<float>();
            o.hueSpeed = 0.0f;
            o.baseColor = (i == 0) ? glm::vec3(1.0f, 0.85f, 0.7f) : glm::vec3(0.6f, 0.75f, 1.0f);
            o.bobHeight = 0.5f;
            o.intensity = 1.0f;
        }
    }

    // boids
    void buildBoids(Scene& scene, const glm::vec3& at) {
        Entity flock = Demo::spawnGroup(scene, "Flock", at);
        flock.addComponent<Flock>();
    }

    // fireworks
    void buildFireworkLauncher(Scene& scene, const glm::vec3& at) {
        Entity pad = Demo::spawnPrimitive(scene, "Firework Launcher", Demo::PrimCube, at + glm::vec3(0, 0.15f, 0),
            glm::vec3(16.0f, 0.3f, 3.0f), glm::vec3(0.15f, 0.12f, 0.1f));
        pad.addComponent<Firework>();
    }
END_SCRIPT(Director)
