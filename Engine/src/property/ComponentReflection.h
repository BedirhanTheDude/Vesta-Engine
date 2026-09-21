#pragma once

#include <property/Property.h>
#include <property/CallbackProperty.h>
#include <property/Payload.h>

class Entity;

class ReflectedComponent : public PropertyHolder, public CallbackPropertyHolder, public PayloadHolder {
public:
	void clear() {
		propertyGroups.clear();
		callbackPropertyGroups.clear();
		payloadGroups.clear();
		fingerprint.clear();
	}

	// the addresses this reflection points at 
	// has to be checked if the fingerprints are still valid (see ComponentReflection::isCurrent)
	std::vector<const void*> fingerprint;
};

// Properties of the built-in components
namespace ComponentReflection {

	// build a reflected component for editor given a componentUID
	bool build(const Entity& entity, unsigned int componentUID, ReflectedComponent& out);

	// true if the fingerprint (the pointers) is pointing at the correct location
	bool isCurrent(const Entity& entity, unsigned int componentUID, const ReflectedComponent& built);
}
