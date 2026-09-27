#pragma once

#include <memory>

class Mesh;

struct MeshData {
	std::shared_ptr<Mesh> mesh;

	unsigned int primitive = 0;
};
