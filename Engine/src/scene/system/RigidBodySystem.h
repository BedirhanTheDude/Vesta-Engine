#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <scene/components/RigidBody.h>
#include <scene/components/Transform.h>
#include <scene/components/MeshData.h>

#include <memory>

class Archive;
class PhysicsWorld;
class PhysicsMaterial;
class Scene;

namespace RigidBodySystem {

	// Creates the physics actor. False (and no body) if the entity is a child, bodies simulate in world space.
	bool attach(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world);
	void detach(RigidBody& rb);

	// This is HEAVY, it rebuilds the entire collider mesh
	void rebuild(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world);
	void recookShape(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world);

	// per physics step: transform -> physics world, then physics world -> transform, both do nothing without a body
	void pushToWorld(RigidBody& rb, const Transform& transform, const MeshData* mesh, PhysicsWorld& world);
	void pullFromWorld(RigidBody& rb, Transform& transform, Scene* scene);

	void setGlobalPose(RigidBody& rb, const glm::vec3& pos, const glm::quat& rot, bool autowake);
	void setKinematicTarget(RigidBody& rb, const glm::vec3& pos, const glm::quat& rot);

	void addForce(RigidBody& rb, const glm::vec3& force);
	void addForceAtPosition(RigidBody& rb, const glm::vec3& force, const glm::vec3& worldPos);
	void addForceAtLocalPosition(RigidBody& rb, const glm::vec3& force, const glm::vec3& localPos);
	void addImpulse(RigidBody& rb, const glm::vec3& impulse);

	void setLinearVelocity(RigidBody& rb, const glm::vec3& v);
	glm::vec3 getLinearVelocity(const RigidBody& rb);
	void setAngularVelocity(RigidBody& rb, const glm::vec3& v);
	glm::vec3 getAngularVelocity(const RigidBody& rb);

	void setLinearDamping(RigidBody& rb, float damping);
	void setAngularDamping(RigidBody& rb, float damping);

	glm::vec3 getWorldPosition(const RigidBody& rb);
	glm::quat getWorldRotation(const RigidBody& rb);

	void setStaticFriction(RigidBody& rb, float f);
	void setDynamicFriction(RigidBody& rb, float f);
	void setRestitution(RigidBody& rb, float r);
	float getStaticFriction(const RigidBody& rb);
	float getDynamicFriction(const RigidBody& rb);
	float getRestitution(const RigidBody& rb);

	void serialize(const RigidBody& rb, Archive& arch);
	void deserialize(RigidBody& rb, const Archive& arch);
}
