#pragma once

#include <memory>
#include <renderer/Mesh.h>

namespace PrimitiveFactory {
	std::shared_ptr<Mesh> createCube();
	std::shared_ptr<Mesh> createPlane();
	std::shared_ptr<Mesh> createSphere(int stacks = 16, int sectors = 32);
}