#include "PhysicsBody.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>

using namespace physx;

PhysicsBody::PhysicsBody(PhysicsWorld* world, std::shared_ptr<PhysicsMaterial> mat, std::shared_ptr<CollisionShape> shape) {
	this->m_World = world;
	this->m_Shape = shape; // handle null pointer later

	if (!mat) mat = world->getDefaultMaterial();
	this->m_Mat = mat;

	this->m_Mat->getOrCreate(world);
}

PhysicsBody::~PhysicsBody() {
	cleanup();
}

glm::vec3 PhysicsBody::getPosition() const {
	auto pxTransform = m_Actor->getGlobalPose();
	return toGlm(pxTransform.p);
}

glm::vec3 PhysicsBody::getRotationEuler() const {
	auto pxTransform = m_Actor->getGlobalPose();
	return glm::eulerAngles(toGlm(pxTransform.q));
}

glm::quat PhysicsBody::getRotation() const {
	auto pxTransform = m_Actor->getGlobalPose();
	return toGlm(pxTransform.q);
}

void PhysicsBody::cleanup() {
	if (!m_Actor) return;

	m_World->removeActor(*m_Actor);
	m_Actor->release();
	m_Actor = nullptr;

	if (m_Mat) {
		m_Mat = nullptr;
	}
}

void PhysicsBody::clearShapes() {
	PxU32 count = m_Actor->getNbShapes();
	std::vector<PxShape*> shapes(count);
	m_Actor->getShapes(shapes.data(), count);
	for (auto* s : shapes)
		m_Actor->detachShape(*s);
}

void PhysicsBody::attachShape(std::shared_ptr<CollisionShape> shape, const glm::vec3& scale) {
	PxShape* pxShape = shape->getOrCreatePxShape(m_World, m_Mat.get(), scale);
	m_Actor->attachShape(*pxShape);
}
