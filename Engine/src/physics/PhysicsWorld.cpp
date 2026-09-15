#include "PhysicsWorld.h"

#include <renderer/Mesh.h>

void PhysicsWorld::init() {
	gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

	PxTolerancesScale scale;
	gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, scale);
	PxInitExtensions(*gPhysics, nullptr);

	PxSceneDesc sceneDesc(scale);
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
	sceneDesc.bounceThresholdVelocity = 0.5f;

	gDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = gDispatcher;

	gScene = gPhysics->createScene(sceneDesc);

	defaultMaterial = std::make_shared<PhysicsMaterial>();
	defaultMaterial->getOrCreate(this);
}

void PhysicsWorld::shutdown() {
	shapeCache.clear();

	gScene->release();
	gDispatcher->release();
	PxCloseExtensions();
	gPhysics->release();
	gFoundation->release();
}

void PhysicsWorld::step(float dt) {
	gScene->simulate(dt);
	gScene->fetchResults(true);
}

void PhysicsWorld::addActor(PxActor& actor) {
	gScene->addActor(actor);
}

void PhysicsWorld::removeActor(PxActor& actor) {
	gScene->removeActor(actor);
}

std::shared_ptr<CollisionShape> PhysicsWorld::getOrCreateShape(
	const Mesh* mesh, const glm::vec3& scale, bool forceConvex) {

	ShapeCacheKey key{ mesh, forceConvex };

	auto it = shapeCache.find(key);
	if (it != shapeCache.end())
		return it->second;

	std::shared_ptr<CollisionShape> shape;

	if (!forceConvex && mesh->primitive != MeshPrimitive::None) {
		switch (mesh->primitive) {
		case MeshPrimitive::Box:
			shape = std::make_shared<CollisionShape>(
				CollisionShape::boxMesh(glm::vec3(0.5f)));
			break;
		case MeshPrimitive::Sphere:
			shape = std::make_shared<CollisionShape>(
				CollisionShape::sphereMesh(0.5f));
			break;
		case MeshPrimitive::Plane:
			shape = std::make_shared<CollisionShape>(
				CollisionShape::boxMesh(glm::vec3(0.5f, 0.001f, 0.5f)));
			break;
		default:
			break;
		}
	}

	if (!shape)
		shape = std::make_shared<CollisionShape>(
			CollisionShape::convexMesh(mesh->getVertices()));

	shapeCache[key] = shape;
	return shape;
}

void PhysicsWorld::cleanShapeCache() {
	for (auto it = shapeCache.begin(); it != shapeCache.end();) {
		if (it->second.use_count() == 1)
			it = shapeCache.erase(it);
		else
			++it;
	}
}