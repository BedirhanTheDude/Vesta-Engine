#include <scene/components/RigidBodyComponent.h>

#include <persistance/Archive.h>

#include <scene/Entity.h>
#include <scene/Scene.h>
#include <scene/components/ComponentFactory.h>
#include <scene/components/MeshComponent.h>
#include <renderer/Mesh.h>

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>
#include <physics/body/PhysicsBody.h>
#include <physics/body/StaticPhysicsBody.h>
#include <physics/body/DynamicPhysicsBody.h>

RigidBodyComponent::RigidBodyComponent(RigidBodyType type,
    std::shared_ptr<PhysicsMaterial> material,
    std::shared_ptr<CollisionShape> shape,
    bool forceConvex)
    : forceConvex(forceConvex), type(type), material(material), shape(shape) {
}

RigidBodyComponent::~RigidBodyComponent() = default;

bool RigidBodyComponent::onAttach() {
    if (owner->getTransform().getParent() != nullptr) {
        printf("Warning: RigidBodyComponent not supported on child entities.\n");
        return false;
    }

    if (!material)
        material = std::make_shared<PhysicsMaterial>();

    PhysicsWorld& world = owner->getScene().getPhysicsWorld();
    glm::vec3 pos = owner->transform.getPosition();
    glm::quat rot = owner->transform.getRotationQuat();

    glm::vec3 scale = owner->transform.getScale();
    lastScale = scale;

    if (!shape) {
        auto* mc = owner->getComponent<MeshComponent>();
        if (mc && mc->getMesh()) {
            if (useTriangleMesh)
                shape = std::make_shared<CollisionShape>(
                    CollisionShape::triangleMesh(
                        mc->getMesh()->getVertices(),
                        mc->getMesh()->getIndices()));
            else
                shape = world.getOrCreateShape(mc->getMesh(), scale, forceConvex);
        }
        else {
            shape = std::make_shared<CollisionShape>(
                CollisionShape::boxMesh(owner->transform.getScale() * 0.5f));
        }
    }

    if (type == RigidBodyType::Static) {
        body = std::make_unique<StaticPhysicsBody>(&world, pos, rot, material, shape, scale);
    }
    else if (type == RigidBodyType::Dynamic) {
        body = std::make_unique<DynamicPhysicsBody>(&world, pos, rot, material, shape, scale);
    }
    else if (type == RigidBodyType::Kinematic) {
        body = std::make_unique<DynamicPhysicsBody>(&world, pos, rot, material, shape, scale, true);
    }

    owner->getTransform().onBeforeReparent = []() {
        printf("Warning: Cannot reparent entity with RigidBodyComponent.\n");
        return false;
        };

    registerProperties();

    return true;
}

void RigidBodyComponent::onDetach() {
    body.reset();

    owner->getTransform().onBeforeReparent = nullptr;
}

void RigidBodyComponent::pushToWorld() {
    glm::vec3 currentScale = owner->transform.getScale();

    if (currentScale != lastScale) {
        lastScale = currentScale;
        recookShape();
    }

    glm::vec3 pos = owner->transform.getPosition();
    glm::quat rot = owner->transform.getRotationQuat();

    if (type == RigidBodyType::Dynamic) {
        bool wake = owner->transform.physicsDirty;
        static_cast<DynamicPhysicsBody*>(body.get())->setGlobalPose(pos, rot, wake);
    }
    else if (type == RigidBodyType::Kinematic) {
        static_cast<DynamicPhysicsBody*>(body.get())->setKinematicTarget(pos, rot);
    }
}

void RigidBodyComponent::pullFromWorld() {
    if (type == RigidBodyType::Dynamic) {
        owner->transform.setPosition(body->getPosition());
        owner->transform.setRotation(body->getRotation());
    }
    owner->transform.physicsDirty = false;
}

void RigidBodyComponent::recookShape() {
    body->clearShapes();

    auto* mc = owner->getComponent<MeshComponent>();
    if (mc && mc->getMesh()) {
        PhysicsWorld& world = owner->getScene().getPhysicsWorld();
        shape = world.getOrCreateShape(mc->getMesh(), owner->transform.getScale(), forceConvex);
    }

    body->attachShape(shape, owner->transform.getScale());
}

// This is HEAVY
void RigidBodyComponent::rebuild() {
    body.reset();
    onAttach();
}

void RigidBodyComponent::setType(RigidBodyType newType) {
    if (newType == type) return;
    type = newType;
    rebuild();
}

void RigidBodyComponent::setForceConvex(bool value) {
    if (forceConvex == value) return;
    forceConvex = value;
    recookShape();
}

void RigidBodyComponent::setUseTriangleMesh(bool value) {
    if (useTriangleMesh == value) return;
    useTriangleMesh = value;
    recookShape();
}

void RigidBodyComponent::setGlobalPose(const glm::vec3& pos, const glm::quat& rot, bool autowake) {
    if (type == RigidBodyType::Static) {
        if (auto* stat = static_cast<StaticPhysicsBody*>(body.get()))
            stat->setGlobalPose(pos, rot);
    }
    else {
        if (auto* dyn = static_cast<DynamicPhysicsBody*>(body.get()))
            dyn->setGlobalPose(pos, rot, autowake);
    }
}

void RigidBodyComponent::setKinematicTarget(const glm::vec3& pos, const glm::quat& rot) {
    if (type != RigidBodyType::Kinematic) return;
    if (auto* dyn = static_cast<DynamicPhysicsBody*>(body.get()))
        dyn->setKinematicTarget(pos, rot);
}

void RigidBodyComponent::addForce(const glm::vec3& force) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->addForce(force);
}

void RigidBodyComponent::addForceAtPosition(const glm::vec3& force, const glm::vec3& worldPos) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->addForceAtPosition(force, worldPos);
}

void RigidBodyComponent::addForceAtLocalPosition(const glm::vec3& force, const glm::vec3& localPos) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->addForceAtLocalPosition(force, localPos);
}

void RigidBodyComponent::addImpulse(const glm::vec3& impulse) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->addImpulse(impulse);
}

void RigidBodyComponent::setLinearVelocity(const glm::vec3& v) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->setLinearVelocity(v);
}

glm::vec3 RigidBodyComponent::getLinearVelocity() const {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        return dyn->getLinearVelocity();
    return glm::vec3(0.0f);
}

void RigidBodyComponent::setAngularVelocity(const glm::vec3& v) {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        dyn->setAngularVelocity(v);
}

glm::vec3 RigidBodyComponent::getAngularVelocity() const {
    if (auto* dyn = dynamic_cast<DynamicPhysicsBody*>(body.get()))
        return dyn->getAngularVelocity();
    return glm::vec3(0.0f);
}

void RigidBodyComponent::setLinearDamping(float damping) {
    auto* dynamicBody = dynamic_cast<DynamicPhysicsBody*>(body.get());
    if (dynamicBody) {
        dynamicBody->setLinearDamping(damping);
    }
}

void RigidBodyComponent::setAngularDamping(float damping) {
    auto* dynamicBody = dynamic_cast<DynamicPhysicsBody*>(body.get());
    if (dynamicBody) {
        dynamicBody->setAngularDamping(damping);
    }
}

glm::vec3 RigidBodyComponent::getWorldPosition() const {
    if (!body) return glm::vec3(0.0f);
    return body->getPosition();
}

glm::quat RigidBodyComponent::getWorldRotation() const {
    if (!body) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    return body->getRotation();
}

void RigidBodyComponent::setStaticFriction(float f) {
    if (!material) material = std::make_shared<PhysicsMaterial>(f, 0.5f, 0.01f);
    else material->setStaticFriction(f);
}

void RigidBodyComponent::setDynamicFriction(float f) {
    if (!material) material = std::make_shared<PhysicsMaterial>(0.5f, f, 0.01f);
    else material->setDynamicFriction(f);
}

void RigidBodyComponent::setRestitution(float r) {
    if (!material) material = std::make_shared<PhysicsMaterial>(0.5f, 0.5f, r);
    else material->setRestitution(r);
}

float RigidBodyComponent::getStaticFriction() const {
    return material ? material->staticFriction : 0.5f;
}

float RigidBodyComponent::getDynamicFriction() const {
    return material ? material->dynamicFriction : 0.5f;
}

float RigidBodyComponent::getRestitution() const {
    return material ? material->restitution : 0.01f;
}

std::shared_ptr<PhysicsMaterial> RigidBodyComponent::getMaterial() const { return material; }

void RigidBodyComponent::serialize(Archive& arch) const {
    switch (type) {
    case Static:    arch.set("bodyType", std::string("Static")); break;
    case Dynamic:   arch.set("bodyType", std::string("Dynamic")); break;
    case Kinematic: arch.set("bodyType", std::string("Kinematic")); break;
    }
    arch.set("forceConvex", forceConvex);

    if (shape) {
        switch (shape->getType()) {
        case CollisionShapeType::TriangleMesh: arch.set("shapeType", std::string("Triangle")); break;
        case CollisionShapeType::ConvexMesh:   arch.set("shapeType", std::string("Convex")); break;
        case CollisionShapeType::BoxMesh:      arch.set("shapeType", std::string("Box")); break;
        case CollisionShapeType::SphereMesh:   arch.set("shapeType", std::string("Sphere")); break;
        }
    }

    if (material) {
        arch.set("staticFriction", material->staticFriction);
        arch.set("dynamicFriction", material->dynamicFriction);
        arch.set("restitution", material->restitution);
    }
}

void RigidBodyComponent::deserialize(const Archive& arch) {
    std::string bt;
    if (!arch.get("bodyType", bt)) bt = "Dynamic";
    if (bt == "Static") type = Static;
    else if (bt == "Kinematic") type = Kinematic;
    else type = Dynamic;

    if (!arch.get("forceConvex", forceConvex)) forceConvex = false;

    std::string shapeType;
    if (!arch.get("shapeType", shapeType)) shapeType = "Box";
    if (shapeType == "Triangle") {
        useTriangleMesh = true;
    }

    if (arch.has("staticFriction")) {
        float sf, df, rest;
        if (!arch.get("staticFriction", sf)) sf = 0.5f;
        if (!arch.get("dynamicFriction", df)) df = 0.5f;
        if (!arch.get("restitution", rest)) rest = 0.01f;
        material = std::make_shared<PhysicsMaterial>(sf, df, rest);
    }
}

void RigidBodyComponent::registerProperties() {
    auto typeCallback = [this](const void* ptr) {
        RigidBodyType type = *static_cast<const RigidBodyType*>(ptr);
        setType(type);
        };

    auto convexCallback = [this](const void* ptr) {
        bool convex = *static_cast<const bool*>(ptr);
        setForceConvex(convex);
        };

    auto triangleCallback = [this](const void* ptr) {
        bool triangle = *static_cast<const bool*>(ptr);
        setUseTriangleMesh(triangle);
        };

    auto linearDampingCallback = [this](const void* ptr) {
        float linearDamping = *static_cast<const float*>(ptr);
        setLinearDamping(linearDamping);
        };

    auto angularDampingCallback = [this](const void* ptr) {
        float angularDamping = *static_cast<const float*>(ptr);
        setAngularDamping(angularDamping);
        };

    auto dynamicFrictionCallback = [this](const void* ptr) {
        float friction = *static_cast<const float*>(ptr);
        setDynamicFriction(friction);
        };

    auto staticFrictionCallback = [this](const void* ptr) {
        float friction = *static_cast<const float*>(ptr);
        setStaticFriction(friction);
        };

    auto restitutionCallback = [this](const void* ptr) {
        float res = *static_cast<const float*>(ptr);
        setRestitution(res);
        };

    registerCallback("Body Type", CallbackPropertyType::Enum, &this->type, RigidBodyType_t::values(), typeCallback);
    registerCallback("Force Convex", CallbackPropertyType::Bool, &this->forceConvex, convexCallback);
    registerCallback("Use Triangle Mesh", CallbackPropertyType::Bool, &useTriangleMesh, triangleCallback);
    registerCallback("Linear Damping", CallbackPropertyType::Float, &linearDamping, linearDampingCallback);
    registerCallback("Angular Damping", CallbackPropertyType::Float, &angularDamping, angularDampingCallback);
    registerCallback("Dynamic Friction", CallbackPropertyType::Float, &material->dynamicFriction, dynamicFrictionCallback);
    registerCallback("Static Friction", CallbackPropertyType::Float, &material->staticFriction, staticFrictionCallback);
    registerCallback("Restitution", CallbackPropertyType::Float, &material->restitution, restitutionCallback);
}

REGISTER(RigidBodyComponent);