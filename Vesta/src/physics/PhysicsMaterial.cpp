#include "PhysicsMaterial.h"

#include <physics/PhysicsWorld.h>

PhysicsMaterial::~PhysicsMaterial() {
	m_PxMat = nullptr;
}

physx::PxMaterial* PhysicsMaterial::getOrCreate(PhysicsWorld* world) {
	if (!m_PxMat && world)
		m_PxMat = world->getPhysics()->createMaterial(staticFriction, dynamicFriction, restitution);
	else if (!m_PxMat && !world) return nullptr;
	
	return m_PxMat;
}

void PhysicsMaterial::setStaticFriction(float sf) {
	staticFriction = sf;
	if (m_PxMat) m_PxMat->setStaticFriction(sf);
}

void PhysicsMaterial::setDynamicFriction(float df) {
	dynamicFriction = df;
	if (m_PxMat) m_PxMat->setDynamicFriction(df);
}

void PhysicsMaterial::setRestitution(float res) {
	restitution = res;
	if (m_PxMat) m_PxMat->setRestitution(res);
}