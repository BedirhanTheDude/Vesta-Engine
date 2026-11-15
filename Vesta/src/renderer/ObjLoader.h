#pragma once

#include <renderer/Mesh.h>
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

struct LoadedModel;
struct Vertex;
class Material;

// the binary caches live in <project>/.vesta/meshes
class ObjLoader {
public:
	static std::shared_ptr<Mesh> load(const std::string& relativePath, bool forceLoadNew = false);
	static LoadedModel loadModel(const std::string& relativePath);
private:
	static void saveCache(const std::filesystem::path& cachePath, const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices);
	static bool loadCache(const std::filesystem::path& cachePath, std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices);
	static void saveModelCache(const std::filesystem::path& cachePath, const std::vector<Vertex>& vertices,
		const std::vector<unsigned int>& indices, const std::vector<SubMesh>& submeshes);
	static bool loadModelCache(const std::filesystem::path& cachePath, std::vector<Vertex>& vertices,
		std::vector<unsigned int>& indices, std::vector<SubMesh>& submeshes);
};