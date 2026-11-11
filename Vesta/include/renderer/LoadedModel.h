#pragma once

#include <vector>
#include <memory>

#include <renderer/Mesh.h>
#include <renderer/Material.h>

struct LoadedModel {
	std::shared_ptr<Mesh> mesh;
	std::vector<std::shared_ptr<Material>> materials;
};