#include <scene/components/MaterialComponent.h>

#include <scene/Entity.h>

#include <core/AssetManager.h>
#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <renderer/Material.h>
#include <renderer/ShaderProgram.h>
#include <renderer/Texture.h>
#include <renderer/Mesh.h>
#include <renderer/LoadedModel.h>

#include <scene/components/MeshComponent.h>

REGISTER(MaterialComponent);

MaterialComponent::MaterialComponent(std::shared_ptr<Material> material) {
    if (material) materials.push_back(material);
    else {
        auto shader = AssetManager::getShader("lit");
        auto mat = std::make_shared<Material>(shader);
        materials.push_back(std::move(mat));
    }

    registerMaterialProperties(0);
    registerMaterialPayloads(0);
}

Material* MaterialComponent::getMaterial(int index) const {
    if (index < 0 || index >= static_cast<int>(materials.size()))
        return nullptr;
    return materials[index].get();
}

void MaterialComponent::serialize(Archive& arch) const {
    for (const auto& mat : materials) {
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

void MaterialComponent::deserialize(const Archive& arch) {
    materials.clear();

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

            materials.push_back(mat);
        }
    }
    else {
        auto* mc = owner->getComponent<MeshComponent>();
        if (mc && mc->getMesh() && !mc->getMesh()->getName().empty()) {
            auto model = AssetManager::getModel(mc->getMesh()->getName());
            if (!model.materials.empty())
                materials = model.materials;
        }

        if (materials.empty()) {
            auto shader = AssetManager::getShader("lit");
            materials.push_back(std::make_shared<Material>(shader));
        }
    }

    rebuildMaterialRegistration();
}

void MaterialComponent::setMaterial(int index, std::shared_ptr<Material> mat) {
    if (index < 0 || index >= (int)materials.size()) return;
    materials[index] = mat;

    auto& groups = getPropertyGroups();
    if ((size_t)index < groups.size())
        groups[index].reset();

    registerMaterialProperties(index);
    registerMaterialPayloads(index);
}

void MaterialComponent::setMaterials(const std::vector<std::shared_ptr<Material>>& mats) {
    materials = mats;

    getPropertyGroups().clear();
    getPayloadGroups().clear();

    for (unsigned int i = 0; i < (unsigned int)materials.size(); ++i) {
        registerMaterialProperties(i);
        registerMaterialPayloads(i);
    }
}

void MaterialComponent::addMaterial(std::shared_ptr<Material> mat) {
    materials.push_back(std::move(mat));

    unsigned int groupIdx = (unsigned int)materials.size() - 1;
    registerMaterialProperties(groupIdx);
    registerMaterialPayloads(groupIdx);
}

void MaterialComponent::addDefaultMaterial() {
    auto shader = AssetManager::getShader("lit");
    materials.push_back(std::make_shared<Material>(shader));

    unsigned int groupIdx = (unsigned int)materials.size() - 1;
    registerMaterialProperties(groupIdx);
    registerMaterialPayloads(groupIdx);
}

void MaterialComponent::registerMaterialProperties(unsigned int materialIdx) {
    Material* mat = getMaterial(materialIdx);
    std::string groupName = "Material " + std::to_string(materialIdx + 1);

    auto alphaCallback = [mat]() {
        mat->transparent = mat->alpha < 1.0f;
        };

    registerProperty("Albedo", PropertyType::Color, &mat->albedo, nullptr, materialIdx, groupName);
    registerProperty("Emission", PropertyType::Color, &mat->emission, nullptr, materialIdx, groupName);
    registerProperty("Shininess", PropertyType::Float, &mat->shininess, nullptr, materialIdx, groupName);
    registerProperty("Ambient Ref", PropertyType::Float, &mat->ambientReflectance, nullptr, materialIdx, groupName);
    registerProperty("Specular Ref", PropertyType::Float, &mat->specularReflectance, nullptr, materialIdx, groupName);
    registerProperty("Alpha", PropertyType::Float, &mat->alpha, alphaCallback, materialIdx, groupName);
}

void MaterialComponent::registerMaterialPayloads(unsigned int materialIdx) {
    Material* mat = getMaterial(materialIdx);
    std::string groupName = "Material " + std::to_string(materialIdx + 1);
    registerPayload("Texture", PayloadType::Texture, &mat->diffuseTexture, materialIdx, groupName);
}

void MaterialComponent::rebuildMaterialRegistration() {
    getPropertyGroups().clear();
    getPayloadGroups().clear();
    for (unsigned int i = 0; i < (unsigned int)materials.size(); ++i) {
        registerMaterialProperties(i);
        registerMaterialPayloads(i);
    }
}