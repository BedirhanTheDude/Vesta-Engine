#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>

class PhysicsWorld;
class PhysicsMaterial;
class CollisionShape;

namespace physx {
	class PxRigidActor;
	class PxMaterial;
}

class PhysicsBody {
public:
	PhysicsBody(PhysicsWorld* world, std::shared_ptr<PhysicsMaterial> mat = nullptr, std::shared_ptr<CollisionShape> shape = nullptr);
	virtual ~PhysicsBody();
	PhysicsBody(const PhysicsBody&) = delete;
	PhysicsBody& operator=(const PhysicsBody&) = delete;

	glm::vec3 getPosition() const;
	glm::vec3 getRotationEuler() const;
	glm::quat getRotation() const;

	void clearShapes();
	void attachShape(std::shared_ptr<CollisionShape> shape, const glm::vec3& scale);
protected:
	virtual void cleanup();

	physx::PxRigidActor* actor = nullptr;
	std::shared_ptr<PhysicsMaterial> mat;
	std::shared_ptr<CollisionShape> shape;
	PhysicsWorld* world;
};