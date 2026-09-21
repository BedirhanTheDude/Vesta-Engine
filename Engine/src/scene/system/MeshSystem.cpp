#include "MeshSystem.h"

#include <core/AssetManager.h>
#include <persistance/Archive.h>

#include <renderer/Mesh.h>
#include <renderer/LoadedModel.h>

#include <vector>

namespace MeshSystem {

    static const std::vector<const char*>& primitiveNames() {
        static const std::vector<const char*> names = { "none", "cube", "sphere", "plane" };
        return names;
    }

    void setMesh(MeshData& data, std::shared_ptr<Mesh> mesh) {
        data.mesh = std::move(mesh);
        data.primitive = 0;
    }

    void setPrimitive(MeshData& data, unsigned int primitive) {
        if (primitive >= primitiveNames().size()) return;

        data.primitive = primitive;
        data.mesh = AssetManager::getMesh(primitiveNames()[primitive]);
    }

    void serialize(const MeshData& data, Archive& arch) {
        if (!data.mesh) arch.set("mesh", "");
        else {
            if (data.mesh->getSubMeshCount() > 1) arch.set("model", data.mesh->getName());
            else arch.set("mesh", data.mesh->getName());
        }

        arch.set("primitive", data.primitive);
    }

    void deserialize(MeshData& data, const Archive& arch) {
        std::string meshName;
        if (arch.get("mesh", meshName)) {
            data.mesh = AssetManager::getMesh(meshName);
        }
        else {
            std::string modelName;
            if (arch.get("model", modelName)) {
                auto model = AssetManager::getModel(modelName);
                data.mesh = model.mesh;
            }
        }

        arch.get("primitive", data.primitive);
    }
}
