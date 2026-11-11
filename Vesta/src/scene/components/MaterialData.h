#pragma once

#include <memory>
#include <vector>

class Material;

struct MaterialData {
	std::vector<std::shared_ptr<Material>> materials;
};
