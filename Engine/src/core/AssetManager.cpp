#include <core/AssetManager.h>

#include <string>

#include <renderer/ObjLoader.h>
#include <renderer/PrimitiveFactory.h>
#include <renderer/ShaderProgram.h>
#include <renderer/Texture.h>

#include <renderer/LoadedModel.h>
#include <renderer/ObjLoader.h>

std::unordered_map<std::string, std::shared_ptr<Mesh>> AssetManager::meshes;
std::unordered_map<std::string, LoadedModel> AssetManager::models;
std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> AssetManager::shaders;
std::unordered_map<std::string, std::shared_ptr<Texture>> AssetManager::textures;

bool AssetManager::loadMesh(const std::string& name) {
    auto it = meshes.find(name);
    if (it != meshes.end()) return true; // already exists

    std::shared_ptr<Mesh> mesh;

    if (name == "cube")
        mesh = PrimitiveFactory::createCube();
    else if (name == "sphere")
        mesh = PrimitiveFactory::createSphere();
    else if (name == "plane")
        mesh = PrimitiveFactory::createPlane();
    else
        mesh = ObjLoader::load(name);

    if (mesh) {
        mesh->setName(name);
        meshes[name] = mesh;
        return true;
    }
    else return false;
}

bool AssetManager::loadShader(const std::string& name) {
    auto it = shaders.find(name);
    if (it != shaders.end()) return true;

    auto shader = std::make_shared<ShaderProgram>(name);
    
    if (shader) {
        shader->setName(name);
        shaders[name] = shader;
        return true;
    }
    else return false;
}

bool AssetManager::loadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) return true;

    auto tex = std::make_shared<Texture>(name);

    if (tex) {
        tex->setName(name);
        textures[name] = tex;
        return true;
    }
    else return false;
}

bool AssetManager::loadModel(const std::string& name) {
    auto it = models.find(name);
    if (it != models.end()) return true;

    LoadedModel model = ObjLoader::loadModel(name);

    if (model.mesh) {
        model.mesh->setName(name);
        models[name] = model;
        meshes[name] = model.mesh;
        return true;
    }
    else return true;
}

std::shared_ptr<Mesh> AssetManager::getMesh(const std::string& name) {
    bool loaded = loadMesh(name);
    
    if (loaded) return meshes[name];
    else return nullptr;
}

std::shared_ptr<ShaderProgram> AssetManager::getShader(const std::string& name) {
    bool loaded = loadShader(name);

    if (loaded) return shaders[name];
    else return nullptr;
}

std::shared_ptr<Texture> AssetManager::getTexture(const std::string& name) {
    bool loaded = loadTexture(name);

    if (loaded) return textures[name];
    else return nullptr;
}

LoadedModel AssetManager::getModel(const std::string& name) {
    bool loaded = loadModel(name);

    if (loaded) return models[name];
    else return {};
}

void AssetManager::clear() {
    meshes.clear();
    models.clear();
    shaders.clear();
    textures.clear();
}