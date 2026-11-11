#include "PhysicsWorld.h"

#include <renderer/Mesh.h>

void PhysicsWorld::init() {
	m_GFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, m_GAllocator, m_GErrorCallback);

	PxTolerancesScale scale;
	m_GPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *m_GFoundation, scale);
	PxInitExtensions(*m_GPhysics, nullptr);

	PxSceneDesc sceneDesc(scale);
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;
	sceneDesc.flags |= PxSceneFlag::eENABLE_STABILIZATION;
	sceneDesc.bounceThresholdVelocity = 0.5f;

	m_GDispatcher = PxDefaultCpuDispatcherCreate(2);
	sceneDesc.cpuDispatcher = m_GDispatcher;

	m_GScene = m_GPhysics->createScene(sceneDesc);

	m_DefaultMaterial = std::make_shared<PhysicsMaterial>();
	m_DefaultMaterial->getOrCreate(this);
}

void PhysicsWorld::shutdown() {
	m_ShapeCache.clear();

	m_GScene->release();
	m_GDispatcher->release();
	PxCloseExtensions();
	m_GPhysics->release();
	m_GFoundation->release();
}

void PhysicsWorld::step(float dt) {
	m_GScene->simulate(dt);
	m_GScene->fetchResults(true);
}

void PhysicsWorld::addActor(PxActor& actor) {
	m_GScene->addActor(actor);
}

void PhysicsWorld::removeActor(PxActor& actor) {
	m_GScene->removeActor(actor);
}

std::shared_ptr<CollisionShape> PhysicsWorld::getOrCreateShape(
	const Mesh* mesh, const glm::vec3& scale, bool forceConvex) {

	ShapeCacheKey key{ mesh, forceConvex };

	auto it = m_ShapeCache.find(key);
	if (it != m_ShapeCache.end())
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

	m_ShapeCache[key] = shape;
	return shape;
}

void PhysicsWorld::cleanShapeCache() {
	for (auto it = m_ShapeCache.begin(); it != m_ShapeCache.end();) {
		if (it->second.use_count() == 1)
			it = m_ShapeCache.erase(it);
		else
			++it;
	}
}