#include <scene/components/MaterialComponent.h>

#include <scene/Entity.h>

#include <scene/components/MaterialData.h>
#include <scene/components/MeshData.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/MaterialSystem.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <renderer/Material.h>

REGISTER(MaterialComponent);

void MaterialComponent::init(std::shared_ptr<Material> material) {
    MaterialData* data = resolveComponent<MaterialData>(entity);
    if (!data) return;

    if (material) MaterialSystem::addMaterial(*data, std::move(material));
    else MaterialSystem::addDefaultMaterial(*data);
}

void MaterialComponent::init(std::vector<std::shared_ptr<Material>> mats) {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::setMaterials(*data, mats);
}

size_t MaterialComponent::getMaterialCount() const {
    MaterialData* data = resolveComponent<MaterialData>(entity);
    return data ? data->materials.size() : 0;
}

Material* MaterialComponent::getMaterial(int index) const {
    MaterialData* data = resolveComponent<MaterialData>(entity);
    return data ? MaterialSystem::getMaterial(*data, index) : nullptr;
}

void MaterialComponent::setMaterial(int index, std::shared_ptr<Material> mat) {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::setMaterial(*data, index, std::move(mat));
}

void MaterialComponent::setMaterials(const std::vector<std::shared_ptr<Material>>& materials) {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::setMaterials(*data, materials);
}

void MaterialComponent::addMaterial(std::shared_ptr<Material> mat) {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::addMaterial(*data, std::move(mat));
}

void MaterialComponent::addDefaultMaterial() {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::addDefaultMaterial(*data);
}

bool MaterialComponent::onAttach() {
    MaterialData* data = resolveComponent<MaterialData>(entity);
    if (!data) return false;

    if (data->materials.empty())
        MaterialSystem::addDefaultMaterial(*data);

    return true;
}

void MaterialComponent::serialize(Archive& arch) const {
    if (MaterialData* data = resolveComponent<MaterialData>(entity))
        MaterialSystem::serialize(*data, arch);
}

void MaterialComponent::deserialize(const Archive& arch) {
    MaterialData* data = resolveComponent<MaterialData>(entity);
    if (!data) return;

    MeshData* meshData = resolveComponent<MeshData>(entity);

    MaterialSystem::deserialize(*data, arch, meshData ? meshData->mesh.get() : nullptr);
}
