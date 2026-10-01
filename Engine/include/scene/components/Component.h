#pragma once

#include <scene/Entity.h>
#include <scene/components/ComponentTypeUID.h>

#include <persistance/Serializable.h>

class Component : ISerializable {
public:
	Component() = default;
	explicit Component(const Entity& entity) : entity(entity) {}

	Entity getEntity() const { return entity; }

protected:
	Entity entity;
};
