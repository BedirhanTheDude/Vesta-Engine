#pragma once

#include <scene/components/MaterialData.h>

#include <memory>
#include <vector>

class Archive;
class Material;
class Mesh;

namespace MaterialSystem {

	// nullptr if index is out of range
	Material* getMaterial(const MaterialData& data, int index);

	void setMaterial(MaterialData& data, int index, std::shared_ptr<Material> mat);
	void setMaterials(MaterialData& data, const std::vector<std::shared_ptr<Material>>& materials);
	void addMaterial(MaterialData& data, std::shared_ptr<Material> mat);
	void addDefaultMaterial(MaterialData& data);

	void serialize(const MaterialData& data, Archive& arch);

	// fallbackMesh: when the archive holds no materials, the materials of the model this mesh
	// came from are used instead (nullptr is fine, a default material is used then)
	void deserialize(MaterialData& data, const Archive& arch, const Mesh* fallbackMesh);
}
