#include "CollisionShape.h"

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <PxPhysicsAPI.h>

using namespace physx;

CollisionShape::~CollisionShape() {
    if (cachedConvexMesh) cachedConvexMesh->release();
    if (cachedTriangleMesh) cachedTriangleMesh->release();
}

CollisionShape CollisionShape::boxMesh(const glm::vec3& halfExtents) {
    CollisionShape s;
    s.type = CollisionShapeType::BoxMesh;
    s.halfExtents = halfExtents;
    return s;
}

CollisionShape CollisionShape::sphereMesh(float radius) {
    CollisionShape s;
    s.type = CollisionShapeType::SphereMesh;
    s.radius = radius;
    return s;
}

CollisionShape CollisionShape::convexMesh(const std::vector<Vertex>& meshVertices) {
    CollisionShape s;
    s.type = CollisionShapeType::ConvexMesh;
    s.vertices.reserve(meshVertices.size());
    for (const Vertex& v : meshVertices)
        s.vertices.push_back(v.position);
    return s;
}

CollisionShape CollisionShape::triangleMesh(const std::vector<Vertex>& meshVertices,
    const std::vector<unsigned int>& meshIndices) {
    CollisionShape s;
    s.type = CollisionShapeType::TriangleMesh;
    s.vertices.reserve(meshVertices.size());
    for (const Vertex& v : meshVertices)
        s.vertices.push_back(v.position);
    s.indices = meshIndices;
    return s;
}

PxShape* CollisionShape::getOrCreatePxShape(PhysicsWorld* world, PhysicsMaterial* mat,
    const glm::vec3& scale) {
    PxPhysics* physics = world->getPhysics();
    PxMaterial* pxMat = mat->getOrCreate(world);
    if (!pxMat) return nullptr;

    switch (type) {
    case CollisionShapeType::BoxMesh: {
        return physics->createShape(
            PxBoxGeometry(toPx(halfExtents * scale)), *pxMat, true);
    }

    case CollisionShapeType::SphereMesh: {
        float r = radius * glm::max(scale.x, glm::max(scale.y, scale.z));
        return physics->createShape(PxSphereGeometry(r), *pxMat, true);
    }

    case CollisionShapeType::ConvexMesh: {
        if (!cachedConvexMesh)
            cachedConvexMesh = cookConvexMesh(world);
        if (!cachedConvexMesh) return nullptr;

        PxMeshScale meshScale(toPx(scale));
        return physics->createShape(
            PxConvexMeshGeometry(cachedConvexMesh, meshScale), *pxMat, true);
    }

    case CollisionShapeType::TriangleMesh: {
        if (!cachedTriangleMesh)
            cachedTriangleMesh = cookTriangleMesh(world);
        if (!cachedTriangleMesh) return nullptr;

        PxMeshScale meshScale(toPx(scale));
        return physics->createShape(
            PxTriangleMeshGeometry(cachedTriangleMesh, meshScale), *pxMat, true);
    }
    }
    return nullptr;
}

PxConvexMesh* CollisionShape::cookConvexMesh(PhysicsWorld* world) {
    PxPhysics* physics = world->getPhysics();

    PxConvexMeshDesc desc;
    desc.points.count = static_cast<PxU32>(vertices.size());
    desc.points.stride = sizeof(PxVec3);
    desc.points.data = vertices.data();
    desc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
    desc.vertexLimit = 255;
    desc.polygonLimit = 255;
    desc.quantizedCount = 255;

    PxDefaultMemoryOutputStream buf;
    PxCookingParams params(physics->getTolerancesScale());

    if (!PxCookConvexMesh(params, desc, buf)) {
        printf("Convex cooking failed! Vertex count: %d\n", (int)vertices.size());
        return nullptr;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    return physics->createConvexMesh(input);
}

PxTriangleMesh* CollisionShape::cookTriangleMesh(PhysicsWorld* world) {
    PxPhysics* physics = world->getPhysics();

    PxTriangleMeshDesc desc;
    desc.points.count = static_cast<PxU32>(vertices.size());
    desc.points.stride = sizeof(glm::vec3);
    desc.points.data = vertices.data();
    desc.triangles.count = static_cast<PxU32>(indices.size() / 3);
    desc.triangles.stride = 3 * sizeof(unsigned int);
    desc.triangles.data = indices.data();

    PxDefaultMemoryOutputStream buf;
    PxCookingParams params(physics->getTolerancesScale());

    if (!PxCookTriangleMesh(params, desc, buf)) {
        printf("Triangle mesh cooking failed! Vertex count: %d\n", (int)vertices.size());
        return nullptr;
    }

    PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
    return physics->createTriangleMesh(input);
}