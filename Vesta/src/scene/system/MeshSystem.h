#pragma once

#include <scene/components/MeshData.h>

#include <memory>

class Archive;
class Mesh;

namespace MeshSystem {

	void setMesh(MeshData& data, std::shared_ptr<Mesh> mesh);

	// selects one of the built-in primitives (none, cube, sphere, plane), out of range is ignored
	void setPrimitive(MeshData& data, unsigned int primitive);

	void serialize(const MeshData& data, Archive& arch);
	void deserialize(MeshData& data, const Archive& arch);
}
