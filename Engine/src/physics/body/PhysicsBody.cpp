#include "PhysicsBody.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>

using namespace physx;

PhysicsBody::PhysicsBody(PhysicsWorld* world, std::shared_ptr<PhysicsMaterial> mat, std::shared_ptr<CollisionShape> shape) {
	this->world = world;
	this->shape = shape; // handle null pointer later

	if (!mat) mat = world->getDefaultMaterial();
	this->mat = mat;

	this->mat->getOrCreate(world);
}

PhysicsBody::~PhysicsBody() {
	cleanup();
}

glm::vec3 PhysicsBody::getPosition() const {
	auto pxTransform = actor->getGlobalPose();
	return toGlm(pxTransform.p);
}

glm::vec3 PhysicsBody::getRotationEuler() const {
	auto pxTransform = actor->getGlobalPose();
	return glm::eulerAngles(toGlm(pxTransform.q));
}

glm::quat PhysicsBody::getRotation() const {
	auto pxTransform = actor->getGlobalPose();
	return toGlm(pxTransform.q);
}

void PhysicsBody::cleanup() {
	if (!actor) return;

	world->removeActor(*actor);
	actor->release();
	actor = nullptr;

	if (mat) {
		mat = nullptr;
	}
}

void PhysicsBody::clearShapes() {
	PxU32 count = actor->getNbShapes();
	std::vector<PxShape*> shapes(count);
	actor->getShapes(shapes.data(), count);
	for (auto* s : shapes)
		actor->detachShape(*s);
}

void PhysicsBody::attachShape(std::shared_ptr<CollisionShape> shape, const glm::vec3& scale) {
	PxShape* pxShape = shape->getOrCreatePxShape(world, mat.get(), scale);
	actor->attachShape(*pxShape);
}
