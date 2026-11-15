#pragma once

#include <scene/Entity.h>
#include <scene/components/ComponentTypeUID.h>

#include <persistence/Serializable.h>

class Component : ISerializable {
public:
	Component() = default;
    explicit Component(const Entity& entity) : entity(entity) {}
    Component(Component&&) noexcept = default;
    Component& operator=(Component&&) noexcept = default;

	Entity getEntity() const { return entity; }

protected:
	Entity entity;
};
