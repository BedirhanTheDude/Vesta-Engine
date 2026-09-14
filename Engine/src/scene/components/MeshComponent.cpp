#include <scene/components/MeshComponent.h>

#include <scene/components/MaterialComponent.h>

#include <scene/Entity.h>
#include <core/AssetManager.h>

#include <scene/components/ComponentFactory.h>
#include <persistance/Archive.h>

#include <renderer/Mesh.h>
#include <renderer/LoadedModel.h>

#include <vector>

REGISTER(MeshComponent);

MeshComponent::MeshComponent(std::shared_ptr<Mesh> mesh) : mesh(mesh) {
    static std::vector<const char*> primNames = { "none", "cube", "sphere", "plane"};

    primitive = 0;
    auto primitiveCallback = [this](const void* ptr) {
        primitive = *static_cast<const unsigned int*>(ptr);
        this->mesh = AssetManager::getMesh(primNames.at(primitive));
        };

    registerCallback("Primitive", CallbackPropertyType::Enum, &primitive, primNames, primitiveCallback);
    registerPayload("Mesh", PayloadType::Mesh, &this->mesh);
}

void MeshComponent::setMesh(std::shared_ptr<Mesh> mesh) {
    this->mesh = mesh;
    primitive = 0;
}

Mesh* MeshComponent::getMesh() {
    return mesh.get();
}

const Mesh* MeshComponent::getMesh() const {
    return mesh.get();
}

void MeshComponent::serialize(Archive& arch) const {
    if (!mesh) arch.set("mesh", "");
    else {
        if (mesh->getSubMeshCount() > 1) arch.set("model", mesh->getName());
        else arch.set("mesh", mesh->getName());
    }

    arch.set("primitive", primitive);
}

void MeshComponent::deserialize(const Archive& arch) {
    std::string meshName;
    if (arch.get("mesh", meshName)) {
        mesh = AssetManager::getMesh(meshName);
    }
    else {
        std::string modelName;
        if (arch.get("model", modelName)) {
            auto model = AssetManager::getModel(modelName);
            mesh = model.mesh;
        }
    }

    arch.get("primitive", primitive);
}