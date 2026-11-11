#include <scene/components/MeshComponent.h>

#include <scene/Entity.h>

#include <scene/components/MeshData.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/MeshSystem.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <renderer/Mesh.h>

REGISTER(MeshComponent);

void MeshComponent::init(std::shared_ptr<Mesh> mesh) {
    if (MeshData* data = resolveComponent<MeshData>(entity))
        MeshSystem::setMesh(*data, std::move(mesh));
}

void MeshComponent::setMesh(std::shared_ptr<Mesh> mesh) {
    if (MeshData* data = resolveComponent<MeshData>(entity))
        MeshSystem::setMesh(*data, std::move(mesh));
}

Mesh* MeshComponent::getMesh() const {
    MeshData* data = resolveComponent<MeshData>(entity);
    return data ? data->mesh.get() : nullptr;
}

void MeshComponent::setPrimitive(unsigned int primitive) {
    if (MeshData* data = resolveComponent<MeshData>(entity))
        MeshSystem::setPrimitive(*data, primitive);
}

unsigned int MeshComponent::getPrimitive() const {
    MeshData* data = resolveComponent<MeshData>(entity);
    return data ? data->primitive : 0;
}

void MeshComponent::serialize(Archive& arch) const {
    if (MeshData* data = resolveComponent<MeshData>(entity))
        MeshSystem::serialize(*data, arch);
}

void MeshComponent::deserialize(const Archive& arch) {
    if (MeshData* data = resolveComponent<MeshData>(entity))
        MeshSystem::deserialize(*data, arch);
}
