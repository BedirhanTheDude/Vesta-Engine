#pragma once

#include <string>

class Scene;

class SceneSerializer {
public:
    static bool save(const Scene& scene, bool temp = false);
    static bool load(Scene& scene, bool temp = false);
};
