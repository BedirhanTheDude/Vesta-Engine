#pragma once

#include <string>
#include <vector>

#include <glm/fwd.hpp>

struct Vertex;

struct SubMesh {
	unsigned int indexOffset;
	unsigned int indexCount;
	int materialIndex;
};

enum class MeshPrimitive { None, Box, Sphere, Plane };

class Mesh {
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
    ~Mesh();

	void draw() const;
	void drawSubMesh(int index) const;

	const std::vector<Vertex>& getVertices() const;
	const std::vector<unsigned int>& getIndices() const;

	int getSubMeshCount() const { return submeshes.empty() ? 1 : submeshes.size(); }
	const std::vector<SubMesh>& getSubMeshes() const { return submeshes; }
	void setSubMeshes(const std::vector<SubMesh>& subs) { submeshes = subs; }

	MeshPrimitive primitive = MeshPrimitive::None;

	std::string getName() const { return name; }
	void setName(const std::string& name) { this->name = name; }

	glm::vec3 computeCentroid() const;

private:
    void setupMesh();

	std::string name = "";
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<SubMesh> submeshes;

    unsigned int VAO = 0;
    unsigned int VBO = 0;
    unsigned int EBO = 0;
};
