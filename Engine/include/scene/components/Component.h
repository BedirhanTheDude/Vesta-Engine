#pragma once

#include <scene/Entity.h>
#include <scene/components/ComponentTypeUID.h>

class Component {
public:
	Component() = default;
	explicit Component(const Entity& entity) : entity(entity) {}

	Entity getEntity() const { return entity; }

protected:
	Entity entity;
};
