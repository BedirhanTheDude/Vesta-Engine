#include "PhysicsMaterial.h"

#include <physics/PhysicsWorld.h>

PhysicsMaterial::~PhysicsMaterial() {
	pxMat = nullptr;
}

physx::PxMaterial* PhysicsMaterial::getOrCreate(PhysicsWorld* world) {
	if (!pxMat && world)
		pxMat = world->getPhysics()->createMaterial(staticFriction, dynamicFriction, restitution);
	else if (!pxMat && !world) return nullptr;
	
	return pxMat;
}

void PhysicsMaterial::setStaticFriction(float sf) {
	staticFriction = sf;
	if (pxMat) pxMat->setStaticFriction(sf);
}

void PhysicsMaterial::setDynamicFriction(float df) {
	dynamicFriction = df;
	if (pxMat) pxMat->setDynamicFriction(df);
}

void PhysicsMaterial::setRestitution(float res) {
	restitution = res;
	if (pxMat) pxMat->setRestitution(res);
}