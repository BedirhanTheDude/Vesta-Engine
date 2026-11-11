#pragma once

#include <physics/body/PhysicsBody.h>

class PhysicsWorld;
class PhysicsMaterial;
class CollisionShape;

class StaticPhysicsBody : public PhysicsBody {
public:
	StaticPhysicsBody(PhysicsWorld* world, const glm::vec3& pos,
					  const glm::quat& rot, std::shared_ptr<PhysicsMaterial> mat = nullptr,
					  std::shared_ptr<CollisionShape> shape = nullptr, const glm::vec3 & scale = glm::vec3(1.0f));
	~StaticPhysicsBody() = default;

	void setGlobalPose(const glm::vec3& pos, const glm::quat& rot);
};