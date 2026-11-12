#include <scene/components/BehaviourComponent.h>

#include <scene/components/ComponentFactory.h>
#include <property/Property.h>
#include <property/PropertySerializer.h>
#include <persistence/Archive.h>

void BehaviourComponent::serialize(Archive& arch) const {
    PropertySerializer::serialize(*(PropertyHolder*)this, arch);
}

void BehaviourComponent::deserialize(const Archive& arch) {
    PropertySerializer::deserialize(*(PropertyHolder*)this, arch);
}