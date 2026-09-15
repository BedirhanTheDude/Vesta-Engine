#pragma once 

#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <renderer/Vertex.h>
#include <PxPhysicsAPI.h>

class PhysicsWorld;
class PhysicsMaterial;

enum class CollisionShapeType { BoxMesh, SphereMesh, ConvexMesh, TriangleMesh };

class CollisionShape {
public:
	~CollisionShape();

	static CollisionShape boxMesh(const glm::vec3& halfExtents);
	static CollisionShape sphereMesh(float radius);
	static CollisionShape convexMesh(const std::vector<Vertex>& meshVertices);
	static CollisionShape triangleMesh(const std::vector<Vertex>& meshVertices,
		const std::vector<unsigned int>& meshIndices);

	physx::PxShape* getOrCreatePxShape(PhysicsWorld* world, PhysicsMaterial* mat,
		const glm::vec3& scale);

	CollisionShapeType getType() const { return type; }

private:
	CollisionShapeType type = CollisionShapeType::BoxMesh;

	glm::vec3 halfExtents = glm::vec3(0.5f);
	float radius = 0.5f;

	std::vector<glm::vec3> vertices;
	std::vector<unsigned int> indices;

	physx::PxConvexMesh* cachedConvexMesh = nullptr;
	physx::PxTriangleMesh* cachedTriangleMesh = nullptr;

	physx::PxConvexMesh* cookConvexMesh(PhysicsWorld* world);
	physx::PxTriangleMesh* cookTriangleMesh(PhysicsWorld* world);
};