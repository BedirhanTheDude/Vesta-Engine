#include "MaterialSystem.h"

#include <core/AssetManager.h>
#include <persistance/Archive.h>

#include <renderer/Material.h>
#include <renderer/ShaderProgram.h>
#include <renderer/Texture.h>
#include <renderer/Mesh.h>
#include <renderer/LoadedModel.h>

namespace MaterialSystem {

    Material* getMaterial(const MaterialData& data, int index) {
        if (index < 0 || index >= static_cast<int>(data.materials.size()))
            return nullptr;
        return data.materials[index].get();
    }

    void setMaterial(MaterialData& data, int index, std::shared_ptr<Material> mat) {
        if (index < 0 || index >= static_cast<int>(data.materials.size())) return;
        data.materials[index] = std::move(mat);
    }

    void setMaterials(MaterialData& data, const std::vector<std::shared_ptr<Material>>& materials) {
        data.materials = materials;
    }

    void addMaterial(MaterialData& data, std::shared_ptr<Material> mat) {
        data.materials.push_back(std::move(mat));
    }

    void addDefaultMaterial(MaterialData& data) {
        auto shader = AssetManager::getShader("lit");
        data.materials.push_back(std::make_shared<Material>(shader));
    }

    void serialize(const MaterialData& data, Archive& arch) {
        for (const auto& mat : data.materials) {
            if (!mat) continue;
            Archive mj;

            if (mat->shader)
                mj.set("shader", mat->shader->getName());

            mj.set("albedo", mat->albedo);
            mj.set("emission", mat->emission);
            mj.set("shininess", mat->shininess);
            mj.set("ambientReflectance", mat->ambientReflectance);
            mj.set("specularReflectance", mat->specularReflectance);
            mj.set("alpha", mat->alpha);
            mj.set("transparent", mat->transparent);

            if (mat->diffuseTexture)
                mj.set("diffuseTexture", mat->diffuseTexture->getName());

            arch.append("materials", std::move(mj));
        }
    }

    void deserialize(MaterialData& data, const Archive& arch, const Mesh* fallbackMesh) {
        data.materials.clear();

        size_t count = arch.size("materials");
        if (count > 0) {
            for (size_t i = 0; i < count; i++) {
                Archive mj = arch.at("materials", i);

                std::string shaderName;
                if (!mj.get("shader", shaderName)) shaderName = "lit";
                auto shader = AssetManager::getShader(shaderName);

                glm::vec3 albedo(0.5f);
                glm::vec3 emission(0.0f);
                mj.get("albedo", albedo);
                mj.get("emission", emission);

                auto mat = std::make_shared<Material>(shader, albedo);
                mat->emission = emission;
                if (!mj.get("shininess", mat->shininess)) mat->shininess = 32.0f;
                if (!mj.get("ambientReflectance", mat->ambientReflectance)) mat->ambientReflectance = 0.5f;
                if (!mj.get("specularReflectance", mat->specularReflectance)) mat->specularReflectance = 0.5f;
                if (!mj.get("alpha", mat->alpha)) mat->alpha = 1.0f;
                if (!mj.get("transparent", mat->transparent)) mat->transparent = false;

                std::string texName;
                if (mj.get("diffuseTexture", texName) && !texName.empty())
                    mat->diffuseTexture = AssetManager::getTexture(texName);

                data.materials.push_back(mat);
            }
        }
        else {
            if (fallbackMesh && !fallbackMesh->getName().empty()) {
                auto model = AssetManager::getModel(fallbackMesh->getName());
                if (!model.materials.empty())
                    data.materials = model.materials;
            }

            if (data.materials.empty())
                addDefaultMaterial(data);
        }
    }
}
