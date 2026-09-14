#include "DynamicPhysicsBody.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

using namespace physx;

DynamicPhysicsBody::DynamicPhysicsBody(PhysicsWorld* world, const glm::vec3& pos,
									   const glm::quat& rot,
									   std::shared_ptr<PhysicsMaterial> mat,
									   std::shared_ptr<CollisionShape> shape, const glm::vec3& scale, bool kinematic)
	: PhysicsBody(world, mat, shape), _isKinematic(kinematic) {

	linearDamping = 0.2f;
	angularDamping = 0.1f;

	PxPhysics* physics = world->getPhysics();
	PxMaterial* pxMat = mat.get()->getOrCreate();

	PxTransform pxTransform(toPx(pos), toPx(rot));

	actor = physics->createRigidDynamic(pxTransform);
	PxShape* pxShape = shape->getOrCreatePxShape(world, this->mat.get(), scale);
	if (!pxShape) {
		printf("Failed to create shape!\n");
		return;
	}
	actor->attachShape(*pxShape);

	static_cast<PxRigidDynamic*>(actor)->setLinearDamping(linearDamping);
	static_cast<PxRigidDynamic*>(actor)->setAngularDamping(angularDamping);
	static_cast<PxRigidDynamic*>(actor)->setSleepThreshold(0.005f);

	if (_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);
	else
		PxRigidBodyExt::updateMassAndInertia(*static_cast<PxRigidDynamic*>(actor), 1.0f);

	world->addActor(*actor);
}

void DynamicPhysicsBody::setGlobalPose(const glm::vec3& pos, const glm::quat& rot, bool autowake) {
	actor->setGlobalPose(PxTransform(toPx(pos), toPx(rot)), autowake);
}

void DynamicPhysicsBody::setKinematicTarget(const glm::vec3& pos, const glm::quat& rot) {
	if (_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->setKinematicTarget(PxTransform(toPx(pos), toPx(rot)));
}

void DynamicPhysicsBody::addForce(const glm::vec3& force) {
	if (!_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->addForce(toPx(force));
}

void DynamicPhysicsBody::addForceAtPosition(const glm::vec3& force, const glm::vec3& worldPos) {
	if (!_isKinematic)
		PxRigidBodyExt::addForceAtPos(
			*static_cast<PxRigidDynamic*>(actor), toPx(force), toPx(worldPos));
}

void DynamicPhysicsBody::addForceAtLocalPosition(const glm::vec3& force, const glm::vec3& localPos) {
	if (!_isKinematic)
		PxRigidBodyExt::addForceAtLocalPos(
			*static_cast<PxRigidDynamic*>(actor), toPx(force), toPx(localPos));
}

void DynamicPhysicsBody::addImpulse(const glm::vec3& impulse) {
	if (!_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->addForce(toPx(impulse), PxForceMode::eIMPULSE);
}

void DynamicPhysicsBody::setLinearVelocity(const glm::vec3& vel) {
	if (!_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->setLinearVelocity(toPx(vel));
}

void DynamicPhysicsBody::setAngularVelocity(const glm::vec3& vel) {
	if (!_isKinematic)
		static_cast<PxRigidDynamic*>(actor)->setAngularVelocity(toPx(vel));
}

void DynamicPhysicsBody::setLinearDamping(float damping) {
	linearDamping = damping;
	static_cast<PxRigidDynamic*>(actor)->setLinearDamping(damping);
}

void DynamicPhysicsBody::setAngularDamping(float damping) {
	angularDamping = damping;
	static_cast<PxRigidDynamic*>(actor)->setAngularDamping(damping);
}

glm::vec3 DynamicPhysicsBody::getLinearVelocity() const {
	if (_isKinematic) return glm::vec3(0.0f);

	return toGlm(static_cast<PxRigidDynamic*>(actor)->getLinearVelocity());
}

glm::vec3 DynamicPhysicsBody::getAngularVelocity() const {
	if (_isKinematic) return glm::vec3(0.0f);

	return toGlm(static_cast<PxRigidDynamic*>(actor)->getAngularVelocity());
}