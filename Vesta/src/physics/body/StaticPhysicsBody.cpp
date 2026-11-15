#include "StaticPhysicsBody.h"

#include <memory>

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

StaticPhysicsBody::StaticPhysicsBody(PhysicsWorld* world, const glm::vec3& pos,
	const glm::quat& rot, std::shared_ptr<PhysicsMaterial> mat,
	std::shared_ptr<CollisionShape> shape, const glm::vec3& scale) : PhysicsBody(world, mat, shape) {
	
	PxPhysics* physics = world->getPhysics();
	PxMaterial* pxMat = this->m_Mat->getOrCreate();

	PxTransform pxTransform(toPx(pos), toPx(rot));

	m_Actor = physics->createRigidStatic(pxTransform);
	PxShape* pxShape = shape->getOrCreatePxShape(world, this->m_Mat.get(), scale);
	if (!pxShape) {
		printf("Failed to create shape!\n");
		return;
	}
	m_Actor->attachShape(*pxShape);

	world->addActor(*m_Actor);
}

void StaticPhysicsBody::setGlobalPose(const glm::vec3& pos, const glm::quat& rot) {
	m_Actor->setGlobalPose(PxTransform(toPx(pos), toPx(rot)), false);
}