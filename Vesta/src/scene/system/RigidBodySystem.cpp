#include "RigidBodySystem.h"

#include <scene/system/TransformSystem.h>

#include <persistance/Archive.h>

#include <renderer/Mesh.h>

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>
#include <physics/body/PhysicsBody.h>
#include <physics/body/StaticPhysicsBody.h>
#include <physics/body/DynamicPhysicsBody.h>

#include <cstdio>

namespace RigidBodySystem {

    bool attach(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world) {
        if (!transform.parent.isInvalid()) {
            printf("Warning: RigidBodyComponent not supported on child entities.\n");
            return false;
        }

        if (!rb.material)
            rb.material = std::make_shared<PhysicsMaterial>();

        glm::vec3 pos = transform.position;
        glm::quat rot = transform.rotation;

        glm::vec3 scale = transform.scale;
        rb.lastScale = scale;

        if (!rb.shape) {
            if (mesh && mesh->mesh) {
                if (rb.useTriangleMesh)
                    rb.shape = std::make_shared<CollisionShape>(
                        CollisionShape::triangleMesh(
                            mesh->mesh->getVertices(),
                            mesh->mesh->getIndices()));
                else
                    rb.shape = world.getOrCreateShape(mesh->mesh.get(), scale, rb.forceConvex);
            }
            else {
                rb.shape = std::make_shared<CollisionShape>(
                    CollisionShape::boxMesh(scale * 0.5f));
            }
        }

        if (rb.type == RigidBodyType::Static) {
            rb.body = std::make_unique<StaticPhysicsBody>(&world, pos, rot, rb.material, rb.shape, scale);
        }
        else if (rb.type == RigidBodyType::Dynamic) {
            rb.body = std::make_unique<DynamicPhysicsBody>(&world, pos, rot, rb.material, rb.shape, scale);
        }
        else if (rb.type == RigidBodyType::Kinematic) {
            rb.body = std::make_unique<DynamicPhysicsBody>(&world, pos, rot, rb.material, rb.shape, scale, true);
        }

        return true;
    }

    void detach(RigidBody& rb) {
        rb.body.reset();
    }

    void rebuild(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world) {
        rb.body.reset();
        attach(rb, transform, mesh, world);
    }

    void recookShape(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world) {
        if (!rb.body) return;

        rb.body->clearShapes();

        if (mesh && mesh->mesh)
            rb.shape = world.getOrCreateShape(mesh->mesh.get(), transform.scale, rb.forceConvex);

        rb.body->attachShape(rb.shape, transform.scale);
    }

    void pushToWorld(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world) {
        if (!rb.body) return; // attach was rejected

        glm::vec3 currentScale = transform.scale;

        if (currentScale != rb.lastScale) {
            rb.lastScale = currentScale;
            recookShape(rb, transform, mesh, world);
        }

        glm::vec3 pos = transform.position;
        glm::quat rot = transform.rotation;

        // checked: type and body are only kept consistent by setType()/attach()
        auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get());
        if (!dyn) return;

        if (rb.type == RigidBodyType::Dynamic) {
            bool wake = transform.physicsDirty;
            dyn->setGlobalPose(pos, rot, wake);
        }
        else if (rb.type == RigidBodyType::Kinematic) {
            dyn->setKinematicTarget(pos, rot);
        }
    }

    void pullFromWorld(RigidBody& rb, Transform& transform, Scene* scene) {
        if (!rb.body || rb.type != RigidBodyType::Dynamic) return;

        TransformSystem::setPosition(transform, rb.body->getPosition(), scene);
        TransformSystem::setRotation(transform, rb.body->getRotation(), scene);
        transform.physicsDirty = false;
    }

    void setGlobalPose(RigidBody& rb, const glm::vec3& pos, const glm::quat& rot, bool autowake) {
        if (rb.type == RigidBodyType::Static) {
            if (auto* stat = dynamic_cast<StaticPhysicsBody*>(rb.body.get()))
                stat->setGlobalPose(pos, rot);
        }
        else {
            if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
                dyn->setGlobalPose(pos, rot, autowake);
        }
    }

    void setKinematicTarget(RigidBody& rb, const glm::vec3& pos, const glm::quat& rot) {
        if (rb.type != RigidBodyType::Kinematic) return;
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->setKinematicTarget(pos, rot);
    }

    void addForce(RigidBody& rb, const glm::vec3& force) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->addForce(force);
    }

    void addForceAtPosition(RigidBody& rb, const glm::vec3& force, const glm::vec3& worldPos) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->addForceAtPosition(force, worldPos);
    }

    void addForceAtLocalPosition(RigidBody& rb, const glm::vec3& force, const glm::vec3& localPos) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->addForceAtLocalPosition(force, localPos);
    }

    void addImpulse(RigidBody& rb, const glm::vec3& impulse) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->addImpulse(impulse);
    }

    void setLinearVelocity(RigidBody& rb, const glm::vec3& v) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->setLinearVelocity(v);
    }

    glm::vec3 getLinearVelocity(const RigidBody& rb) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            return dyn->getLinearVelocity();
        return glm::vec3(0.0f);
    }

    void setAngularVelocity(RigidBody& rb, const glm::vec3& v) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->setAngularVelocity(v);
    }

    glm::vec3 getAngularVelocity(const RigidBody& rb) {
        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            return dyn->getAngularVelocity();
        return glm::vec3(0.0f);
    }

    void setLinearDamping(RigidBody& rb, float damping) {
        rb.linearDamping = damping;

        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->setLinearDamping(damping);
    }

    void setAngularDamping(RigidBody& rb, float damping) {
        rb.angularDamping = damping;

        if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(rb.body.get()))
            dyn->setAngularDamping(damping);
    }

    glm::vec3 getWorldPosition(const RigidBody& rb) {
        if (!rb.body) return glm::vec3(0.0f);
        return rb.body->getPosition();
    }

    glm::quat getWorldRotation(const RigidBody& rb) {
        if (!rb.body) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        return rb.body->getRotation();
    }

    void setStaticFriction(RigidBody& rb, float f) {
        if (!rb.material) rb.material = std::make_shared<PhysicsMaterial>(f, 0.5f, 0.01f);
        else rb.material->setStaticFriction(f);
    }

    void setDynamicFriction(RigidBody& rb, float f) {
        if (!rb.material) rb.material = std::make_shared<PhysicsMaterial>(0.5f, f, 0.01f);
        else rb.material->setDynamicFriction(f);
    }

    void setRestitution(RigidBody& rb, float r) {
        if (!rb.material) rb.material = std::make_shared<PhysicsMaterial>(0.5f, 0.5f, r);
        else rb.material->setRestitution(r);
    }

    float getStaticFriction(const RigidBody& rb) {
        return rb.material ? rb.material->staticFriction : 0.5f;
    }

    float getDynamicFriction(const RigidBody& rb) {
        return rb.material ? rb.material->dynamicFriction : 0.5f;
    }

    float getRestitution(const RigidBody& rb) {
        return rb.material ? rb.material->restitution : 0.01f;
    }

    void serialize(const RigidBody& rb, Archive& arch) {
        switch (rb.type) {
        case Static:    arch.set("bodyType", std::string("Static")); break;
        case Dynamic:   arch.set("bodyType", std::string("Dynamic")); break;
        case Kinematic: arch.set("bodyType", std::string("Kinematic")); break;
        }
        arch.set("forceConvex", rb.forceConvex);

        if (rb.shape) {
            switch (rb.shape->getType()) {
            case CollisionShapeType::TriangleMesh: arch.set("shapeType", std::string("Triangle")); break;
            case CollisionShapeType::ConvexMesh:   arch.set("shapeType", std::string("Convex")); break;
            case CollisionShapeType::BoxMesh:      arch.set("shapeType", std::string("Box")); break;
            case CollisionShapeType::SphereMesh:   arch.set("shapeType", std::string("Sphere")); break;
            }
        }

        if (rb.material) {
            arch.set("staticFriction", rb.material->staticFriction);
            arch.set("dynamicFriction", rb.material->dynamicFriction);
            arch.set("restitution", rb.material->restitution);
        }
    }

    void deserialize(RigidBody& rb, const Archive& arch) {
        std::string bt;
        if (!arch.get("bodyType", bt)) bt = "Dynamic";
        if (bt == "Static") rb.type = Static;
        else if (bt == "Kinematic") rb.type = Kinematic;
        else rb.type = Dynamic;

        if (!arch.get("forceConvex", rb.forceConvex)) rb.forceConvex = false;

        std::string shapeType;
        if (!arch.get("shapeType", shapeType)) shapeType = "Box";
        if (shapeType == "Triangle") {
            rb.useTriangleMesh = true;
        }

        if (arch.has("staticFriction")) {
            float sf, df, rest;
            if (!arch.get("staticFriction", sf)) sf = 0.5f;
            if (!arch.get("dynamicFriction", df)) df = 0.5f;
            if (!arch.get("restitution", rest)) rest = 0.01f;
            rb.material = std::make_shared<PhysicsMaterial>(sf, df, rest);
        }
    }
}
