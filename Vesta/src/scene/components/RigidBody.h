#pragma once

#include <scene/components/RigidBodyComponent.h>

#include <physics/body/PhysicsBody.h>

#include <glm/glm.hpp>
#include <memory>

class PhysicsMaterial;
class CollisionShape;

struct RigidBody {
	RigidBodyType type = Dynamic;

	bool forceConvex = false;
	bool useTriangleMesh = false;

	float linearDamping = 0.2f;
	float angularDamping = 0.1f;

	glm::vec3 lastScale = glm::vec3(1.0f);

	std::unique_ptr<PhysicsBody> body;
	std::shared_ptr<PhysicsMaterial> material;
	std::shared_ptr<CollisionShape> shape;
};
