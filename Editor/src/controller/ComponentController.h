#pragma once

#include <property/Property.h>
#include <property/Payload.h>
#include <property/CallbackProperty.h>

#include <map>
#include <vector>
#include <string>

namespace ComponentController {
	void createAndBindComponent(const std::string& componentName, unsigned int entityID);
	void removeComponent(unsigned int entityID, unsigned int componentUID);
	bool entityHasComponent(unsigned int entityID, const std::string& componentName);
	
	bool canPasteComponent();
	bool copyTransform(unsigned int entityID); // Of course Transform is special, what did you think?
	bool copyComponent(unsigned int entityID, unsigned int componentUID);
	void pasteComponent(unsigned int entityID);

	const std::vector<std::string>& getAvailableComponentNames();
	const std::vector<std::string>& getAvailableScriptNames();

	// UIDs of the components the entity has (built-ins and scripts, not the transform), by value
	std::vector<unsigned int> getEntityComponentUIDs(unsigned int entityID);

	const std::map<unsigned int, Property>& getComponentProperties(unsigned int entityID, unsigned int componentUID, unsigned int groupID = 0);
	const std::vector<PropertyGroup>& getComponentPropertyGroups(unsigned int entityID, unsigned int componentUID);
	const std::map<unsigned int, Payload>& getComponentPayloads(unsigned int entityID, unsigned int componentUID, unsigned int groupID = 0);
	const std::vector<PayloadGroup>& getComponentPayloadGroups(unsigned int entityID, unsigned int componentUID);
	const std::vector<CallbackPropertyGroup>& getComponentCallbackPropertyGroups(unsigned int entityID, unsigned int componentUID);
	std::string componentUIDToString(unsigned int componentUID);
};
