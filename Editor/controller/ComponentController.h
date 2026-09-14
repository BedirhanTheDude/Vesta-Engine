#pragma once

#include <property/Property.h>
#include <property/Payload.h>
#include <property/CallbackProperty.h>

#include <map>
#include <vector>
#include <string>

class ComponentController {
public:
	static void createAndBindComponent(const std::string& componentName, unsigned int entityID);
	static void removeComponent(unsigned int entityID, unsigned int componentUID);
	static bool entityHasComponent(unsigned int entityID, const std::string& componentName);
	static std::vector<std::string> getAvailableComponentNames();

	static const std::map<unsigned int, Property>& getComponentProperties(unsigned int entityID, unsigned int componentUID, unsigned int groupID = 0);
	static const std::vector<PropertyGroup>& getComponentPropertyGroups(unsigned int entityID, unsigned int componentUID);
	static const std::map<unsigned int, Payload>& getComponentPayloads(unsigned int entityID, unsigned int componentUID, unsigned int groupID = 0);
	static const std::vector<PayloadGroup>& getComponentPayloadGroups(unsigned int entityID, unsigned int componentUID);
	static const std::vector<CallbackPropertyGroup>& getComponentCallbackPropertyGroups(unsigned int entityID, unsigned int componentUID);
	static std::string componentUIDToString(unsigned int componentUID);
};