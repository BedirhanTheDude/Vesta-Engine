#include <renderer/Mesh.h>

#include <renderer/Vertex.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : vertices(vertices), indices(indices)
    {
    name = "";
    setupMesh();
}

Mesh::~Mesh()
{
    if (EBO) glDeleteBuffers(1, &EBO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (VAO) glDeleteVertexArrays(1, &VAO);
}

const std::vector<Vertex>& Mesh::getVertices() const { return vertices; }
const std::vector<unsigned int>& Mesh::getIndices() const { return indices; }

void Mesh::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 vertices.size() * sizeof(Vertex),
                 vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 indices.size() * sizeof(unsigned int),
                 indices.data(),
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                         (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::draw() const
{
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLuint>(indices.size()), GL_UNSIGNED_INT, 0);
}

glm::vec3 Mesh::computeCentroid() const {
    if (vertices.empty()) return glm::vec3(0.0f);
    glm::vec3 sum(0.0f);
    for (const auto& v : vertices)
        sum += v.position;
    return sum / static_cast<float>(vertices.size());
}

void Mesh::drawSubMesh(int index) const {
	glBindVertexArray(VAO);
	const auto& sub = submeshes.at(index);
	glDrawElements(GL_TRIANGLES, sub.indexCount, GL_UNSIGNED_INT,
		(void*)(sub.indexOffset * sizeof(unsigned int)));
	glBindVertexArray(0);
}