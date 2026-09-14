#pragma once

#include <property/Payload.h>
#include <string>

class AssetController {
public:
	static bool loadMesh(const std::string& meshName);
	static bool loadModel(const std::string& modelName);
	static bool loadTexture(const std::string& textureName);

	static bool setMesh(unsigned int entityID, const std::string& meshName);
	static bool setModel(unsigned int entityID, const std::string& modelName);
	static bool setTexture(unsigned int entityID, unsigned int groupIdx, unsigned int payloadIdx, const std::string& textureName);

	static std::string getResourceName(const Payload& payload);
};