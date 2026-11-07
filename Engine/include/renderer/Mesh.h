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

	int getSubMeshCount() const { return m_Submeshes.empty() ? 1 : m_Submeshes.size(); }
	const std::vector<SubMesh>& getSubMeshes() const { return m_Submeshes; }
	void setSubMeshes(const std::vector<SubMesh>& subs) { m_Submeshes = subs; }

	MeshPrimitive primitive = MeshPrimitive::None;

	std::string getName() const { return m_Name; }
	void setName(const std::string& name) { this->m_Name = name; }

	glm::vec3 computeCentroid() const;

private:
    void setupMesh();

	std::string m_Name = "";
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<SubMesh> m_Submeshes;

    unsigned int m_VAO = 0;
    unsigned int m_VBO = 0;
    unsigned int m_EBO = 0;
};
