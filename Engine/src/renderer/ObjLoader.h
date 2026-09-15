#pragma once

#include <renderer/Mesh.h>
#include <memory>
#include <string>
#include <vector>

struct LoadedModel;
struct Vertex;
class Material;

class ObjLoader {
public:
	static std::shared_ptr<Mesh> load(const std::string& filePath, bool forceLoadNew = false);
	static LoadedModel loadModel(const std::string& objName);
private:
	static void saveCache(const std::string& objName, const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices);
	static bool loadCache(const std::string& objName, std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices);
	static void saveModelCache(const std::string& objName, const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, const std::vector<SubMesh>& submeshes);
	static bool loadModelCache(const std::string& objName, std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices, std::vector<SubMesh>& submeshes);
};