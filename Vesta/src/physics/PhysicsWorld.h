#pragma once

#include <PxPhysicsAPI.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <memory>
#include <unordered_map>

#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

using namespace physx;
using namespace glm;

class Mesh;

inline PxVec3 toPx(const vec3& v) { return PxVec3(v.x, v.y, v.z); }
inline vec3 toGlm(const PxVec3& v) { return vec3(v.x, v.y, v.z); }
inline PxQuat toPx(const quat& q) { return PxQuat(q.x, q.y, q.z, q.w); }
inline quat toGlm(const PxQuat& q) { return quat(q.w, q.x, q.y, q.z); }

struct ShapeCacheKey {
	const Mesh* mesh;
	bool forceConvex;
	bool operator==(const ShapeCacheKey& other) const {
		return mesh == other.mesh && forceConvex == other.forceConvex;
	}
};

struct ShapeCacheKeyHash {
	size_t operator()(const ShapeCacheKey& k) const {
		return std::hash<const Mesh*>()(k.mesh) ^ std::hash<bool>()(k.forceConvex);
	}
};

class PhysicsWorld {
public:
	void init();
	void shutdown();
	void step(float dt);

	void addActor(PxActor& actor);
	void removeActor(PxActor& actor);
	void cleanShapeCache();

	PxPhysics* getPhysics() const { return m_GPhysics; }
	PxScene* getScene() const { return m_GScene; }

	std::shared_ptr<PhysicsMaterial> getDefaultMaterial() { return m_DefaultMaterial; }
	std::shared_ptr<CollisionShape> getOrCreateShape(const Mesh* mesh, const glm::vec3& scale, bool forceConvex = false);
private:
	PxDefaultAllocator m_GAllocator;
	PxDefaultErrorCallback m_GErrorCallback;

	PxFoundation* m_GFoundation = nullptr;
	PxPhysics* m_GPhysics = nullptr;
	PxScene* m_GScene = nullptr;
	PxDefaultCpuDispatcher* m_GDispatcher = nullptr;

	std::shared_ptr<PhysicsMaterial> m_DefaultMaterial = nullptr;
	std::unordered_map<ShapeCacheKey, std::shared_ptr<CollisionShape>, ShapeCacheKeyHash> m_ShapeCache;
};