#include "StaticPhysicsBody.h"

#include <memory>

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

StaticPhysicsBody::StaticPhysicsBody(PhysicsWorld* world, const glm::vec3& pos,
	const glm::quat& rot, std::shared_ptr<PhysicsMaterial> mat,
	std::shared_ptr<CollisionShape> shape, const glm::vec3& scale) : PhysicsBody(world, mat, shape) {
	
	PxPhysics* physics = world->getPhysics();
	PxMaterial* pxMat = mat.get()->getOrCreate();

	PxTransform pxTransform(toPx(pos), toPx(rot));

	actor = physics->createRigidStatic(pxTransform);
	PxShape* pxShape = shape->getOrCreatePxShape(world, this->mat.get(), scale);
	if (!pxShape) {
		printf("Failed to create shape!\n");
		return;
	}
	actor->attachShape(*pxShape);

	world->addActor(*actor);
}

void StaticPhysicsBody::setGlobalPose(const glm::vec3& pos, const glm::quat& rot) {
	actor->setGlobalPose(PxTransform(toPx(pos), toPx(rot)), false);
}