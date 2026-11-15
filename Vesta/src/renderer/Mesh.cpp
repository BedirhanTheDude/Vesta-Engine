#include <renderer/Mesh.h>

#include <renderer/Vertex.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices)
    : m_Vertices(vertices), m_Indices(indices)
    {
    m_Name = "";
    setupMesh();
}

Mesh::~Mesh()
{
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

const std::vector<Vertex>& Mesh::getVertices() const { return m_Vertices; }
const std::vector<unsigned int>& Mesh::getIndices() const { return m_Indices; }

void Mesh::setupMesh() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 m_Vertices.size() * sizeof(Vertex),
                 m_Vertices.data(),
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_Indices.size() * sizeof(unsigned int),
                 m_Indices.data(),
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
    glBindVertexArray(m_VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLuint>(m_Indices.size()), GL_UNSIGNED_INT, 0);
}

glm::vec3 Mesh::computeCentroid() const {
    if (m_Vertices.empty()) return glm::vec3(0.0f);
    glm::vec3 sum(0.0f);
    for (const auto& v : m_Vertices)
        sum += v.position;
    return sum / static_cast<float>(m_Vertices.size());
}

void Mesh::drawSubMesh(int index) const {
	glBindVertexArray(m_VAO);
	const auto& sub = m_Submeshes.at(index);
	glDrawElements(GL_TRIANGLES, sub.indexCount, GL_UNSIGNED_INT,
		(void*)(sub.indexOffset * sizeof(unsigned int)));
	glBindVertexArray(0);
}