#pragma once

#include <property/Payload.h>
#include <string>

namespace AssetController {
	bool loadMesh(const std::string& meshName);
	bool loadModel(const std::string& modelName);
	bool loadTexture(const std::string& textureName);

	bool setMesh(unsigned int entityID, const std::string& meshName);
	bool setModel(unsigned int entityID, const std::string& modelName);
	bool setTexture(unsigned int entityID, unsigned int groupIdx, unsigned int payloadIdx, const std::string& textureName);

	std::string getResourceName(const Payload& payload);
};