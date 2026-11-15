#include "DynamicPhysicsBody.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

using namespace physx;

DynamicPhysicsBody::DynamicPhysicsBody(PhysicsWorld* world, const glm::vec3& pos,
									   const glm::quat& rot,
									   std::shared_ptr<PhysicsMaterial> mat,
									   std::shared_ptr<CollisionShape> shape, const glm::vec3& scale, bool kinematic)
	: PhysicsBody(world, mat, shape), m_IsKinematic(kinematic) {

	m_LinearDamping = 0.2f;
	m_AngularDamping = 0.1f;

	PxPhysics* physics = world->getPhysics();
	PxMaterial* pxMat = this->m_Mat->getOrCreate();

	PxTransform pxTransform(toPx(pos), toPx(rot));

	m_Actor = physics->createRigidDynamic(pxTransform);
	PxShape* pxShape = shape->getOrCreatePxShape(world, this->m_Mat.get(), scale);
	if (!pxShape) {
		printf("Failed to create shape!\n");
		return;
	}
	m_Actor->attachShape(*pxShape);

	static_cast<PxRigidDynamic*>(m_Actor)->setLinearDamping(m_LinearDamping);
	static_cast<PxRigidDynamic*>(m_Actor)->setAngularDamping(m_AngularDamping);
	static_cast<PxRigidDynamic*>(m_Actor)->setSleepThreshold(0.005f);

	if (m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	else
		PxRigidBodyExt::updateMassAndInertia(*static_cast<PxRigidDynamic*>(m_Actor), 1.0f);

	world->addActor(*m_Actor);
}

void DynamicPhysicsBody::setGlobalPose(const glm::vec3& pos, const glm::quat& rot, bool autowake) {
	m_Actor->setGlobalPose(PxTransform(toPx(pos), toPx(rot)), autowake);
}

void DynamicPhysicsBody::setKinematicTarget(const glm::vec3& pos, const glm::quat& rot) {
	if (m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->setKinematicTarget(PxTransform(toPx(pos), toPx(rot)));
}

void DynamicPhysicsBody::addForce(const glm::vec3& force) {
	if (!m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->addForce(toPx(force));
}

void DynamicPhysicsBody::addForceAtPosition(const glm::vec3& force, const glm::vec3& worldPos) {
	if (!m_IsKinematic)
		PxRigidBodyExt::addForceAtPos(
			*static_cast<PxRigidDynamic*>(m_Actor), toPx(force), toPx(worldPos));
}

void DynamicPhysicsBody::addForceAtLocalPosition(const glm::vec3& force, const glm::vec3& localPos) {
	if (!m_IsKinematic)
		PxRigidBodyExt::addForceAtLocalPos(
			*static_cast<PxRigidDynamic*>(m_Actor), toPx(force), toPx(localPos));
}

void DynamicPhysicsBody::addImpulse(const glm::vec3& impulse) {
	if (!m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->addForce(toPx(impulse), PxForceMode::eIMPULSE);
}

void DynamicPhysicsBody::setLinearVelocity(const glm::vec3& vel) {
	if (!m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->setLinearVelocity(toPx(vel));
}

void DynamicPhysicsBody::setAngularVelocity(const glm::vec3& vel) {
	if (!m_IsKinematic)
		static_cast<PxRigidDynamic*>(m_Actor)->setAngularVelocity(toPx(vel));
}

void DynamicPhysicsBody::setLinearDamping(float damping) {
	m_LinearDamping = damping;
	static_cast<PxRigidDynamic*>(m_Actor)->setLinearDamping(damping);
}

void DynamicPhysicsBody::setAngularDamping(float damping) {
	m_AngularDamping = damping;
	static_cast<PxRigidDynamic*>(m_Actor)->setAngularDamping(damping);
}

glm::vec3 DynamicPhysicsBody::getLinearVelocity() const {
	if (m_IsKinematic) return glm::vec3(0.0f);

	return toGlm(static_cast<PxRigidDynamic*>(m_Actor)->getLinearVelocity());
}

glm::vec3 DynamicPhysicsBody::getAngularVelocity() const {
	if (m_IsKinematic) return glm::vec3(0.0f);

	return toGlm(static_cast<PxRigidDynamic*>(m_Actor)->getAngularVelocity());
}