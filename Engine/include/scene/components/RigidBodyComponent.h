#pragma once

#include <scene/components/Component.h>

#include <utility/EnumToString.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

class PhysicsMaterial;
class CollisionShape;
class Archive;

enum RigidBodyType : int
{
	Static,
	Dynamic,
	Kinematic
};

struct RigidBodyType_t {
	static const char* toString(RigidBodyType type) {
		switch (type) {
			ENUM_TO_STRING(Static);
			ENUM_TO_STRING(Dynamic);
			ENUM_TO_STRING(Kinematic);
		}
		return "Unknown";
	}

	static std::vector<const char*>& values() {
		static std::vector<const char*> values = {
			toString(Static),
			toString(Dynamic),
			toString(Kinematic)
		};
		return values;
	}
};

class RigidBodyComponent : public Component {
public:
	explicit RigidBodyComponent(const Entity& entity) : Component(entity) {}

	// what addComponent<RigidBodyComponent>(type, material, shape, forceConvex) forwards to
	void init(RigidBodyType type = Dynamic,
		std::shared_ptr<PhysicsMaterial> material = nullptr,
		std::shared_ptr<CollisionShape> shape = nullptr,
		bool forceConvex = false);

	// onAttach creates the physics actor, false if the entity has a parent (bodies simulate in world space)
	bool onAttach();
	void onDetach();

	void serialize(Archive& arch) const;
	void deserialize(const Archive& arch);

	RigidBodyType getType() const;
	void setType(RigidBodyType newType);

	bool getForceConvex() const;
	void setForceConvex(bool value);

	bool getUseTriangleMesh() const;
	void setUseTriangleMesh(bool value);

	void setGlobalPose(const glm::vec3& pos, const glm::quat& rot, bool autowake = true);
	void setKinematicTarget(const glm::vec3& pos, const glm::quat& rot);

	void addForce(const glm::vec3& force);
	void addForceAtPosition(const glm::vec3& force, const glm::vec3& worldPos);
	void addForceAtLocalPosition(const glm::vec3& force, const glm::vec3& localPos);
	void addImpulse(const glm::vec3& impulse);

	void setLinearVelocity(const glm::vec3& v);
	glm::vec3 getLinearVelocity() const;

	void setAngularVelocity(const glm::vec3& v);
	glm::vec3 getAngularVelocity() const;

	void setLinearDamping(float damping);
	void setAngularDamping(float damping);
	float getLinearDamping() const;
	float getAngularDamping() const;

	glm::vec3 getWorldPosition() const;
	glm::quat getWorldRotation() const;

	void setStaticFriction(float f);
	void setDynamicFriction(float f);
	void setRestitution(float r);
	float getStaticFriction() const;
	float getDynamicFriction() const;
	float getRestitution() const;

	std::shared_ptr<PhysicsMaterial> getMaterial() const;
};
