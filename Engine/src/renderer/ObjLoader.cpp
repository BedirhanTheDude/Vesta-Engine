#include "ObjLoader.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_object_loader.h>

#pragma warning(pop)

#include <filesystem>
#include <renderer/Vertex.h>
#include <iostream>
#include <fstream>
#include <unordered_map>

#include <core/AssetManager.h>
#include <renderer/Material.h>
#include <renderer/LoadedModel.h>

struct VertexKey {
    int pos, norm, tex;
    bool operator==(const VertexKey& other) const {
        return pos == other.pos && norm == other.norm && tex == other.tex;
    }
};

struct VertexKeyHash {
    size_t operator()(const VertexKey& k) const {
        size_t h = std::hash<int>()(k.pos);
        h ^= std::hash<int>()(k.norm) << 1;
        h ^= std::hash<int>()(k.tex) << 2;
        return h;
    }
};

std::shared_ptr<Mesh> ObjLoader::load(const std::string& objName, bool forceLoadNew) {

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    if (!forceLoadNew && loadCache(objName, vertices, indices)) {
        return std::make_shared<Mesh>(vertices, indices);
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    std::filesystem::path modelDir = std::filesystem::current_path() / "assets" / "models" / objName;
    std::filesystem::path mtlDir = std::filesystem::current_path() / "assets" / "models";

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, modelDir.u8string().c_str(), mtlDir.u8string().c_str())) {
        std::cerr << "OBJ load failed: " << err << std::endl;
        return nullptr;
    }

    if (!warn.empty()) std::cerr << "OBJ warning: " << warn << std::endl;

    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> uniqueVertices;

    size_t totalIndices = 0;
    for (const auto& shape : shapes)
        totalIndices += shape.mesh.indices.size();

    vertices.reserve(totalIndices);
    indices.reserve(totalIndices);

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            VertexKey key{ index.vertex_index, index.normal_index, index.texcoord_index };

            if (uniqueVertices.count(key)) {
                indices.push_back(uniqueVertices[key]);
                continue;
            }

            Vertex vertex{};

            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };
            } else {
                vertex.normal = { 0.0f, 1.0f, 0.0f };
            }

            if (index.texcoord_index >= 0) {
                vertex.texCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    attrib.texcoords[2 * index.texcoord_index + 1]
                };
            } else {
                vertex.texCoord = { 0.0f, 0.0f };
            }

            unsigned int newIndex = static_cast<unsigned int>(vertices.size());
            uniqueVertices[key] = newIndex;
            indices.push_back(newIndex);
            vertices.push_back(vertex);
        }
    }

    indices.shrink_to_fit();
    vertices.shrink_to_fit();

    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    for (const auto& v : vertices) {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }

    glm::vec3 center = (minBounds + maxBounds) * 0.5f;

    for (auto& v : vertices) {
        v.position -= center;
    }

    glm::vec3 size = maxBounds - minBounds;
    float maxDimension = glm::max(size.x, glm::max(size.y, size.z));
    if (maxDimension > 0.0f) {
        float scale = 1.0f / maxDimension;
        for (auto& v : vertices)
            v.position *= scale;
    }

    saveCache(objName, vertices, indices);
    return std::make_shared<Mesh>(vertices, indices);
}

LoadedModel ObjLoader::loadModel(const std::string& objName) {
    std::filesystem::path filePath = std::filesystem::current_path() / "assets" / "models" / objName;
    std::filesystem::path mtlDir = filePath.parent_path();

    size_t dot = objName.find_last_of(".");
    std::string mtlName;
    if (dot != std::string::npos)
        mtlName = objName.substr(0, dot);
    else
        mtlName = objName;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> tinyMaterials;
    std::map<std::string, int> matMap;
    std::string warn, err;

    bool cacheHit;
    bool loaded = false;
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<SubMesh> submeshes;
    std::shared_ptr<Mesh> mesh = nullptr;

    cacheHit = loadModelCache(objName, vertices, indices, submeshes);

    if (cacheHit) {
        mesh = std::make_shared<Mesh>(vertices, indices);
        mesh->setSubMeshes(submeshes);

        std::string mtlFile;
        std::ifstream objStream(filePath);
        std::string line;

        while (std::getline(objStream, line)) {
            if (line.substr(0, 7) == "mtllib ") {
                mtlFile = line.substr(7);
                mtlFile.erase(mtlFile.find_last_not_of(" \t\r\n") + 1);
                break;
            }
        }

        if (!mtlFile.empty()) {
            std::ifstream mtlStream(mtlDir / mtlFile);
            if (mtlStream.is_open())
                tinyobj::LoadMtl(&matMap, &tinyMaterials, &mtlStream, &warn, &err);
        }

        loaded = true;
    }
    else {
        loaded = tinyobj::LoadObj(
            &attrib,
            &shapes,
            &tinyMaterials,
            &warn,
            &err,
            filePath.u8string().c_str(),
            mtlDir.u8string().c_str()
        );
    }

    if (cacheHit)
        std::cout << objName << " loaded from cache" << '\n';

    if (!warn.empty())
        std::cerr << "OBJ warning: " << warn << '\n';

    if (!loaded) {
        std::cerr << "OBJ load failed: " << err << '\n';
        return {};
    }

    auto shader = AssetManager::getShader("lit");

    std::vector<std::shared_ptr<Material>> materials;
    materials.reserve(tinyMaterials.size());

    for (const auto& tmat : tinyMaterials) {
        auto mat = std::make_shared<Material>(
            shader,
            glm::vec3(
                tmat.diffuse[0],
                tmat.diffuse[1],
                tmat.diffuse[2]
            )
        );

        mat->shininess = tmat.shininess > 0.0f ? tmat.shininess : 32.0f;
        mat->specularReflectance = (tmat.specular[0] + tmat.specular[1] + tmat.specular[2]) / 3.0f;
        mat->alpha = tmat.dissolve;
        mat->transparent = tmat.dissolve < 1.0f;
        printf("mat %s dissolve=%.2f transparent=%d\n",
            tmat.name.c_str(), tmat.dissolve, mat->transparent);

        if (!tmat.diffuse_texname.empty()) {
            mat->diffuseTexture = AssetManager::getTexture(tmat.diffuse_texname);
        }

        materials.push_back(mat);
    }

    if (materials.empty()) {
        materials.push_back(std::make_shared<Material>(shader));
    }

    if (cacheHit) return { mesh, materials };

    std::unordered_map<VertexKey, unsigned int, VertexKeyHash> uniqueVertices;

    struct FaceGroup {
        int materialIndex = 0;
        std::vector<unsigned int> indices;
    };

    std::map<int, FaceGroup> groups;

    for (const auto& shape : shapes) {
        size_t indexOffset = 0;

        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
            int fv = shape.mesh.num_face_vertices[f];

            if (fv != 3) {
                indexOffset += fv;
                continue;
            }

            int matId = shape.mesh.material_ids.empty()
                ? 0
                : shape.mesh.material_ids[f];

            if (matId < 0 || matId >= static_cast<int>(materials.size()))
                matId = 0;

            auto& group = groups[matId];
            group.materialIndex = matId;

            for (int v = 0; v < fv; ++v) {
                tinyobj::index_t index = shape.mesh.indices[indexOffset + v];

                VertexKey key{
                    index.vertex_index,
                    index.normal_index,
                    index.texcoord_index
                };

                auto found = uniqueVertices.find(key);
                if (found != uniqueVertices.end()) {
                    group.indices.push_back(found->second);
                    continue;
                }

                Vertex vertex{};

                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                if (index.normal_index >= 0) {
                    vertex.normal = {
                        attrib.normals[3 * index.normal_index + 0],
                        attrib.normals[3 * index.normal_index + 1],
                        attrib.normals[3 * index.normal_index + 2]
                    };
                }
                else {
                    vertex.normal = { 0.0f, 1.0f, 0.0f };
                }

                if (index.texcoord_index >= 0) {
                    vertex.texCoord = {
                        attrib.texcoords[2 * index.texcoord_index + 0],
                        attrib.texcoords[2 * index.texcoord_index + 1]
                    };
                }
                else {
                    vertex.texCoord = { 0.0f, 0.0f };
                }

                unsigned int newIndex =
                    static_cast<unsigned int>(vertices.size());

                uniqueVertices[key] = newIndex;
                group.indices.push_back(newIndex);
                vertices.push_back(vertex);
            }

            indexOffset += fv;
        }
    }

    for (auto& [matId, group] : groups) {
        SubMesh submesh{};
        submesh.indexOffset = static_cast<unsigned int>(indices.size());
        submesh.indexCount = static_cast<unsigned int>(group.indices.size());
        submesh.materialIndex = group.materialIndex;

        submeshes.push_back(submesh);

        indices.insert(
            indices.end(),
            group.indices.begin(),
            group.indices.end()
        );
    }

    glm::vec3 minBounds(FLT_MAX);
    glm::vec3 maxBounds(-FLT_MAX);

    for (const auto& v : vertices) {
        minBounds = glm::min(minBounds, v.position);
        maxBounds = glm::max(maxBounds, v.position);
    }

    glm::vec3 center = (minBounds + maxBounds) * 0.5f;

    for (auto& v : vertices) {
        v.position -= center;
    }

    glm::vec3 size = maxBounds - minBounds;
    float maxDimension = glm::max(size.x, glm::max(size.y, size.z));
    if (maxDimension > 0.0f) {
        float scale = 1.0f / maxDimension;
        for (auto& v : vertices)
            v.position *= scale;
    }

    vertices.shrink_to_fit();
    indices.shrink_to_fit();
    submeshes.shrink_to_fit();

    saveModelCache(objName, vertices, indices, submeshes);

    mesh = std::make_shared<Mesh>(vertices, indices);
    mesh->setSubMeshes(submeshes);

    return { mesh, materials };
}

// Directly write obj data to a binary file once loaded
// because it is faster to read compared to tinyobjloader
void ObjLoader::saveCache(const std::string& objName,
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices) {

    std::filesystem::path cacheDir = std::filesystem::current_path() / "assets" / "models" / "OBJCache";

    std::filesystem::create_directories(cacheDir);
    std::filesystem::path filePath = std::filesystem::current_path() / "assets" / "models" / "OBJCache" / (objName + ".mesh.cache");
    
    std::ofstream file(filePath, std::ios::binary);

    size_t vertCount = vertices.size();
    size_t idxCount = indices.size();

    file.write(reinterpret_cast<const char*>(&vertCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&idxCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(vertices.data()), vertCount * sizeof(Vertex));
    file.write(reinterpret_cast<const char*>(indices.data()), idxCount * sizeof(unsigned int));
}

// Checks if cache exists, if so load vertices and indices and return true, if not return false
bool ObjLoader::loadCache(const std::string& objName,
    std::vector<Vertex>& vertices,
    std::vector<unsigned int>& indices) {

    std::filesystem::path filePath = std::filesystem::current_path() / "assets" / "models" / "OBJCache" / (objName + ".mesh.cache");
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) return false;

    vertices.clear();
    indices.clear();

    size_t vertCount, idxCount;
    file.read(reinterpret_cast<char*>(&vertCount), sizeof(size_t));
    file.read(reinterpret_cast<char*>(&idxCount), sizeof(size_t));

    vertices.resize(vertCount);
    indices.resize(idxCount);

    file.read(reinterpret_cast<char*>(vertices.data()), vertCount * sizeof(Vertex));
    file.read(reinterpret_cast<char*>(indices.data()), idxCount * sizeof(unsigned int));

    return true;
}

void ObjLoader::saveModelCache(const std::string& objName,
                          const std::vector<Vertex>& vertices,
                          const std::vector<unsigned int>& indices,
                          const std::vector<SubMesh>& submeshes) {

    std::filesystem::path cacheDir = std::filesystem::current_path() / "assets" / "models" / "OBJCache";

    std::filesystem::create_directories(cacheDir);
    std::filesystem::path filePath = std::filesystem::current_path() / "assets" / "models" / "OBJCache" / (objName + ".model.cache");
    std::ofstream file(filePath, std::ios::binary);

    size_t vertCount = vertices.size();
    size_t idxCount = indices.size();
    size_t subCount = submeshes.size();

    file.write(reinterpret_cast<const char*>(&vertCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&idxCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(&subCount), sizeof(size_t));
    file.write(reinterpret_cast<const char*>(vertices.data()), vertCount * sizeof(Vertex));
    file.write(reinterpret_cast<const char*>(indices.data()), idxCount * sizeof(unsigned int));
    file.write(reinterpret_cast<const char*>(submeshes.data()), subCount * sizeof(SubMesh));
}

bool ObjLoader::loadModelCache(const std::string& objName,
                          std::vector<Vertex>& vertices,
                          std::vector<unsigned int>& indices,
                          std::vector<SubMesh>& submeshes) {

    std::filesystem::path filePath = std::filesystem::current_path() / "assets" / "models" / "OBJCache" / (objName + ".model.cache");
    std::ifstream file(filePath, std::ios::binary);

    if (!file.is_open()) return false;

    vertices.clear();
    indices.clear();

    size_t vertCount, idxCount, subCount;
    file.read(reinterpret_cast<char*>(&vertCount), sizeof(size_t));
    file.read(reinterpret_cast<char*>(&idxCount), sizeof(size_t));
    file.read(reinterpret_cast<char*>(&subCount), sizeof(size_t));

    vertices.resize(vertCount);
    indices.resize(idxCount);
    submeshes.resize(subCount);

    file.read(reinterpret_cast<char*>(vertices.data()), vertCount * sizeof(Vertex));
    file.read(reinterpret_cast<char*>(indices.data()), idxCount * sizeof(unsigned int));
    file.read(reinterpret_cast<char*>(submeshes.data()), subCount * sizeof(SubMesh));

    return true;
}
