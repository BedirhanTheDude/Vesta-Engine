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

	CollisionShapeType getType() const { return m_Type; }

private:
	CollisionShapeType m_Type = CollisionShapeType::BoxMesh;

	glm::vec3 m_HalfExtents = glm::vec3(0.5f);
	float m_Radius = 0.5f;

	std::vector<glm::vec3> m_Vertices;
	std::vector<unsigned int> m_Indices;

	physx::PxConvexMesh* m_CachedConvexMesh = nullptr;
	physx::PxTriangleMesh* m_CachedTriangleMesh = nullptr;

	physx::PxConvexMesh* cookConvexMesh(PhysicsWorld* world);
	physx::PxTriangleMesh* cookTriangleMesh(PhysicsWorld* world);
};