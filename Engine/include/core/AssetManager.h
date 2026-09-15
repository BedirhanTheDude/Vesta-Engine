#pragma once

#include <memory>
#include <unordered_map>

#include <string>

struct LoadedModel;
class ShaderProgram;
class Mesh;
class Texture;

class AssetManager {
public:
    static bool loadMesh(const std::string& name);
    static bool loadModel(const std::string& name);
    static bool loadShader(const std::string& name);
    static bool loadTexture(const std::string& name);

    static std::shared_ptr<Mesh> getMesh(const std::string& name);
    static LoadedModel getModel(const std::string& name);
    static std::shared_ptr<ShaderProgram> getShader(const std::string& name);
    static std::shared_ptr<Texture> getTexture(const std::string& name);

    static void clear();

private:
    static std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
    static std::unordered_map<std::string, LoadedModel> models;
    static std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> shaders;
    static std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
};