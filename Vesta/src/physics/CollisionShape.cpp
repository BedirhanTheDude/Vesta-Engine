#include "CollisionShape.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <PxPhysicsAPI.h>

using namespace physx;

CollisionShape::~CollisionShape() {
    if (m_CachedConvexMesh) m_CachedConvexMesh->release();
    if (m_CachedTriangleMesh) m_CachedTriangleMesh->release();
}

CollisionShape CollisionShape::boxMesh(const glm::vec3& halfExtents) {
    CollisionShape s;
    s.m_Type = CollisionShapeType::BoxMesh;
    s.m_HalfExtents = halfExtents;
    return s;
}

CollisionShape CollisionShape::sphereMesh(float radius) {
    CollisionShape s;
    s.m_Type = CollisionShapeType::SphereMesh;
    s.m_Radius = radius;
    return s;
}

CollisionShape CollisionShape::convexMesh(const std::vector<Vertex>& meshVertices) {
    CollisionShape s;
    s.m_Type = CollisionShapeType::ConvexMesh;
    s.m_Vertices.reserve(meshVertices.size());
    for (const Vertex& v : meshVertices)
        s.m_Vertices.push_back(v.position);
    return s;
}

CollisionShape CollisionShape::triangleMesh(const std::vector<Vertex>& meshVertices,
    const std::vector<unsigned int>& meshIndices) {
    CollisionShape s;
    s.m_Type = CollisionShapeType::TriangleMesh;
    s.m_Vertices.reserve(meshVertices.size());
    for (const Vertex& v : meshVertices)
        s.m_Vertices.push_back(v.position);
    s.m_Indices = meshIndices;
    return s;
}

PxShape* CollisionShape::getOrCreatePxShape(PhysicsWorld* world, PhysicsMaterial* mat,
    const glm::vec3& scale) {
    PxPhysics* physics = world->getPhysics();
    PxMaterial* pxMat = mat->getOrCreate(world);
    if (!pxMat) return nullptr;

    switch (m_Type) {
    case CollisionShapeType::BoxMesh: {
        return physics->createShape(
            PxBoxGeometry(toPx(m_HalfExtents * scale)), *pxMat, true);
    }

    case CollisionShapeType::SphereMesh: {
        float r = m_Radius * glm::max(scale.x, glm::max(scale.y, scale.z));
        return physics->createShape(PxSphereGeometry(r), *pxMat, true);
    }

    case CollisionShapeType::ConvexMesh: {
        if (!m_CachedConvexMesh)
            m_CachedConvexMesh = cookConvexMesh(world);
        if (!m_CachedConvexMesh) return nullptr;

        PxMeshScale meshScale(toPx(scale));
        return physics->createShape(
            PxConvexMeshGeometry(m_CachedConvexMesh, meshScale), *pxMat, true);
    }

    case CollisionShapeType::TriangleMesh: {
        if (!m_CachedTriangleMesh)
            m_CachedTriangleMesh = cookTriangleMesh(world);
        if (!m_CachedTriangleMesh) return nullptr;

        PxMeshScale meshScale(toPx(scale));
        return physics->createShape(
            PxTriangleMeshGeometry(m_CachedTriangleMesh, meshScale), *pxMat, true);
    }
    }
    return nullptr;
}

PxConvexMesh* CollisionShape::cookConvexMesh(PhysicsWorld* world) {
    PxPhysics* physics = world->getPhysics();

    PxConvexMeshDesc desc;
    desc.points.count = static_cast<PxU32>(m_Vertices.size());
    desc.points.stride = sizeof(PxVec3);
    desc.points.data = m_Vertices.data();
    desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
    desc.vertexLimit = 255;
    desc.polygonLimit = 255;
    desc.quantizedCount = 255;

    PxDefaultMemoryOutputStream buf;
    PxCookingParams params(physics->getTolerancesScale());

    if (!PxCookConvexMesh(params, desc, buf)) {
        printf("Convex cooking failed! Vertex count: %d\n", (int)m_Vertices.size());
        return nullptr;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    return physics->createConvexMesh(input);
}

PxTriangleMesh* CollisionShape::cookTriangleMesh(PhysicsWorld* world) {
    PxPhysics* physics = world->getPhysics();

    PxTriangleMeshDesc desc;
    desc.points.count = static_cast<PxU32>(m_Vertices.size());
    desc.points.stride = sizeof(glm::vec3);
    desc.points.data = m_Vertices.data();
    desc.triangles.count = static_cast<PxU32>(m_Indices.size() / 3);
    desc.triangles.stride = 3 * sizeof(unsigned int);
    desc.triangles.data = m_Indices.data();

    PxDefaultMemoryOutputStream buf;
    PxCookingParams params(physics->getTolerancesScale());

    if (!PxCookTriangleMesh(params, desc, buf)) {
        printf("Triangle mesh cooking failed! Vertex count: %d\n", (int)m_Vertices.size());
        return nullptr;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    return physics->createTriangleMesh(input);
}