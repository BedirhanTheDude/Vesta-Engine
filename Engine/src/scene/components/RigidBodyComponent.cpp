#include <scene/components/RigidBodyComponent.h>

#include <persistance/Archive.h>

#include <scene/Entity.h>
#include <scene/Scene.h>
#include <scene/components/ComponentFactory.h>

#include <scene/components/RigidBody.h>
#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>
#include <scene/components/ProxyUtil.h>
#include <scene/system/RigidBodySystem.h>

#include <physics/PhysicsWorld.h>
#include <physics/PhysicsMaterial.h>
#include <physics/CollisionShape.h>

// everything the physics solver needs from the entity's other components, mesh is null if it has none
struct RigidBodyContext {
    RigidBody* rb;
    Transform* transform;
    MeshData* mesh;
    PhysicsWorld* world;
};

static bool resolveContext(const Entity& entity, RigidBodyContext& context) {
    context.rb = resolveComponent<RigidBody>(entity);
    context.transform = resolveComponent<Transform>(entity);
    if (!context.rb || !context.transform) return false;

    context.mesh = resolveComponent<MeshData>(entity);
    context.world = &entity.getScene().getPhysicsWorld();

    return true;
}

void RigidBodyComponent::init(RigidBodyType type,
    std::shared_ptr<PhysicsMaterial> material,
    std::shared_ptr<CollisionShape> shape,
    bool forceConvex) {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    if (!rb) return;

    rb->type = type;
    rb->material = std::move(material);
    rb->shape = std::move(shape);
    rb->forceConvex = forceConvex;
}

bool RigidBodyComponent::onAttach() {
    RigidBodyContext context;
    if (!resolveContext(entity, context)) return false;

    return RigidBodySystem::attach(*context.rb, *context.transform, context.mesh, *context.world);
}

void RigidBodyComponent::onDetach() {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::detach(*rb);
}

void RigidBodyComponent::serialize(Archive& arch) const {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::serialize(*rb, arch);
}

void RigidBodyComponent::deserialize(const Archive& arch) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::deserialize(*rb, arch);
}

RigidBodyType RigidBodyComponent::getType() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->type : Dynamic;
}

void RigidBodyComponent::setType(RigidBodyType newType) {
    RigidBodyContext context;
    if (!resolveContext(entity, context)) return;

    if (newType == context.rb->type) return;
    context.rb->type = newType;

    RigidBodySystem::rebuild(*context.rb, *context.transform, context.mesh, *context.world);
}

bool RigidBodyComponent::getForceConvex() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->forceConvex : false;
}

void RigidBodyComponent::setForceConvex(bool value) {
    RigidBodyContext context;
    if (!resolveContext(entity, context)) return;

    if (context.rb->forceConvex == value) return;
    context.rb->forceConvex = value;

    RigidBodySystem::recookShape(*context.rb, *context.transform, context.mesh, *context.world);
}

bool RigidBodyComponent::getUseTriangleMesh() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->useTriangleMesh : false;
}

void RigidBodyComponent::setUseTriangleMesh(bool value) {
    RigidBodyContext context;
    if (!resolveContext(entity, context)) return;

    if (context.rb->useTriangleMesh == value) return;
    context.rb->useTriangleMesh = value;

    RigidBodySystem::recookShape(*context.rb, *context.transform, context.mesh, *context.world);
}

void RigidBodyComponent::setGlobalPose(const glm::vec3& pos, const glm::quat& rot, bool autowake) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setGlobalPose(*rb, pos, rot, autowake);
}

void RigidBodyComponent::setKinematicTarget(const glm::vec3& pos, const glm::quat& rot) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setKinematicTarget(*rb, pos, rot);
}

void RigidBodyComponent::addForce(const glm::vec3& force) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::addForce(*rb, force);
}

void RigidBodyComponent::addForceAtPosition(const glm::vec3& force, const glm::vec3& worldPos) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::addForceAtPosition(*rb, force, worldPos);
}

void RigidBodyComponent::addForceAtLocalPosition(const glm::vec3& force, const glm::vec3& localPos) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::addForceAtLocalPosition(*rb, force, localPos);
}

void RigidBodyComponent::addImpulse(const glm::vec3& impulse) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::addImpulse(*rb, impulse);
}

void RigidBodyComponent::setLinearVelocity(const glm::vec3& v) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setLinearVelocity(*rb, v);
}

glm::vec3 RigidBodyComponent::getLinearVelocity() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getLinearVelocity(*rb) : glm::vec3(0.0f);
}

void RigidBodyComponent::setAngularVelocity(const glm::vec3& v) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setAngularVelocity(*rb, v);
}

glm::vec3 RigidBodyComponent::getAngularVelocity() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getAngularVelocity(*rb) : glm::vec3(0.0f);
}

void RigidBodyComponent::setLinearDamping(float damping) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setLinearDamping(*rb, damping);
}

void RigidBodyComponent::setAngularDamping(float damping) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setAngularDamping(*rb, damping);
}

float RigidBodyComponent::getLinearDamping() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->linearDamping : 0.2f;
}

float RigidBodyComponent::getAngularDamping() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->angularDamping : 0.1f;
}

glm::vec3 RigidBodyComponent::getWorldPosition() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getWorldPosition(*rb) : glm::vec3(0.0f);
}

glm::quat RigidBodyComponent::getWorldRotation() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getWorldRotation(*rb) : glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
}

void RigidBodyComponent::setStaticFriction(float f) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setStaticFriction(*rb, f);
}

void RigidBodyComponent::setDynamicFriction(float f) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setDynamicFriction(*rb, f);
}

void RigidBodyComponent::setRestitution(float r) {
    if (RigidBody* rb = resolveComponent<RigidBody>(entity))
        RigidBodySystem::setRestitution(*rb, r);
}

float RigidBodyComponent::getStaticFriction() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getStaticFriction(*rb) : 0.5f;
}

float RigidBodyComponent::getDynamicFriction() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getDynamicFriction(*rb) : 0.5f;
}

float RigidBodyComponent::getRestitution() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? RigidBodySystem::getRestitution(*rb) : 0.01f;
}

std::shared_ptr<PhysicsMaterial> RigidBodyComponent::getMaterial() const {
    RigidBody* rb = resolveComponent<RigidBody>(entity);
    return rb ? rb->material : nullptr;
}

REGISTER(RigidBodyComponent);
