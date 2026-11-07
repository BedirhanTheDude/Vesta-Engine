#include <core/AssetManager.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <system_error>
#include <climits>

#include <core/Project.h>

#include <renderer/ObjLoader.h>
#include <renderer/PrimitiveFactory.h>
#include <renderer/ShaderProgram.h>
#include <renderer/Texture.h>

#include <renderer/LoadedModel.h>
#include <renderer/ObjLoader.h>

std::unordered_map<std::string, std::shared_ptr<Mesh>> AssetManager::m_Meshes;
std::unordered_map<std::string, LoadedModel> AssetManager::m_Models;
std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> AssetManager::m_Shaders;
std::unordered_map<std::string, std::shared_ptr<Texture>> AssetManager::m_Textures;

bool AssetManager::loadMesh(const std::string& name) {
    auto it = m_Meshes.find(name);
    if (it != m_Meshes.end()) return true; // already exists

    std::shared_ptr<Mesh> mesh;

    if (name == "cube")
        mesh = PrimitiveFactory::createCube();
    else if (name == "sphere")
        mesh = PrimitiveFactory::createSphere();
    else if (name == "quad")
        mesh = PrimitiveFactory::createQuad();
    else
        mesh = ObjLoader::load(name);

    if (mesh) {
        mesh->setName(name);
        m_Meshes[name] = mesh;
        return true;
    }
    else return false;
}

static bool resolveShaderBasePath(const std::string& name, std::filesystem::path& outBasePath) {
    std::filesystem::path projectBase;
    bool inProject = Project::resolve(name, projectBase);

    // skip the built-in shader check if path contains "/" (a subfolder)
    if (name.find('/') == std::string::npos) {
        std::filesystem::path engineBase = ShaderProgram::getEngineShaderDirectory() / std::filesystem::u8path(name);
        std::filesystem::path engineVert = engineBase;
        engineVert += ".vert";

        std::error_code ec;
        if (std::filesystem::exists(engineVert, ec)) {
            std::filesystem::path projectVert = projectBase;
            projectVert += ".vert";

            if (inProject && std::filesystem::exists(projectVert, ec))
                std::cerr << "[Warning] Project shader \"" << name
                    << "\" has the same name as a built-in shader, the built-in one is used\n";

            outBasePath = std::move(engineBase);
            return true;
        }
    }

    if (!inProject) return false;

    outBasePath = std::move(projectBase);
    return true;
}

bool AssetManager::loadShader(const std::string& name) {
    auto it = m_Shaders.find(name);
    if (it != m_Shaders.end()) return true;

    std::filesystem::path basePath;
    if (!resolveShaderBasePath(name, basePath)) {
        std::cerr << "[Error] Shader \"" << name << "\" is neither built-in nor a path inside the project\n";
        return false;
    }

    auto shader = std::make_shared<ShaderProgram>(name, basePath);
    m_Shaders[name] = shader;
    return true;
}

bool AssetManager::loadTexture(const std::string& name) {
    auto it = m_Textures.find(name);
    if (it != m_Textures.end()) return true;

    std::filesystem::path filePath;
    if (!Project::resolve(name, filePath)) {
        std::cerr << "[Error] Texture \"" << name << "\" is not a path inside the project\n";
        return false;
    }

    auto tex = std::make_shared<Texture>(filePath);
    if (!tex->isLoaded()) return false;

    tex->setName(name);
    m_Textures[name] = tex;
    return true;
}

bool AssetManager::loadModel(const std::string& name) {
    auto it = m_Models.find(name);
    if (it != m_Models.end()) return true;

    LoadedModel model = ObjLoader::loadModel(name);

    if (model.mesh) {
        model.mesh->setName(name);
        m_Models[name] = model;
        m_Meshes[name] = model.mesh;
        return true;
    }
    else return false;
}

std::shared_ptr<Mesh> AssetManager::getMesh(const std::string& name) {
    bool loaded = loadMesh(name);
    
    if (loaded) return m_Meshes[name];
    else return nullptr;
}

std::shared_ptr<ShaderProgram> AssetManager::getShader(const std::string& name) {
    bool loaded = loadShader(name);

    if (loaded) return m_Shaders[name];
    else return nullptr;
}

std::shared_ptr<Texture> AssetManager::getTexture(const std::string& name) {
    bool loaded = loadTexture(name);

    if (loaded) return m_Textures[name];
    else return nullptr;
}

LoadedModel AssetManager::getModel(const std::string& name) {
    bool loaded = loadModel(name);

    if (loaded) return m_Models[name];
    else return {};
}

void AssetManager::clear() {
    m_Meshes.clear();
    m_Models.clear();
    m_Shaders.clear();
    m_Textures.clear();
}