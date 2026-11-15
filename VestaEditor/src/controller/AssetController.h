#pragma once

#include <property/Payload.h>
#include <string>
#include <filesystem>

namespace AssetController {
	// absolute path of a file inside the project -> its asset name, false if it's outside the project
	bool getAssetName(const std::filesystem::path& file, std::string& outName);

	bool loadMesh(const std::string& meshName);
	bool loadModel(const std::string& modelName);
	bool loadTexture(const std::string& textureName);

	bool setMesh(unsigned int entityID, const std::string& meshName);
	bool setModel(unsigned int entityID, const std::string& modelName);
	bool setTexture(unsigned int entityID, unsigned int groupIdx, unsigned int payloadIdx, const std::string& textureName);

	std::string getResourceName(const Payload& payload);
};