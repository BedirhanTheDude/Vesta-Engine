#include "PrimitiveFactory.h"

#include <vector>
#include <memory>
#include <renderer/Vertex.h>

#include <glm/ext/scalar_constants.hpp>

class Mesh;

namespace PrimitiveFactory
{
    std::shared_ptr<Mesh> PrimitiveFactory::createSphere(int stacks, int sectors) {
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;

        for (int i = 0; i <= stacks; i++) {
            float phi = glm::pi<float>() * (float)i / stacks; // 0 to PI (top to bottom)
            float y = cos(phi);
            float r = sin(phi);

            for (int j = 0; j <= sectors; j++) {
                float theta = 2.0f * glm::pi<float>() * (float)j / sectors; // 0 to 2PI

                float x = r * cos(theta);
                float z = r * sin(theta);

                glm::vec3 pos(x * 0.5f, y * 0.5f, z * 0.5f);
                glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
                glm::vec2 uv((float)j / sectors, (float)i / stacks);

                vertices.push_back({ pos, normal, uv });
            }
        }

        for (int i = 0; i < stacks; i++) {
            for (int j = 0; j < sectors; j++) {
                int topLeft = i * (sectors + 1) + j;
                int topRight = topLeft + 1;
                int bottomLeft = topLeft + (sectors + 1);
                int bottomRight = bottomLeft + 1;

                indices.push_back(topLeft);
                indices.push_back(bottomLeft);
                indices.push_back(topRight);

                indices.push_back(topRight);
                indices.push_back(bottomLeft);
                indices.push_back(bottomRight);
            }
        }

        auto mesh = std::make_shared<Mesh>(vertices, indices);
        mesh->primitive = MeshPrimitive::Sphere;
        return mesh;
    }

	std::shared_ptr<Mesh> createPlane() {
        std::vector<Vertex> vertices =
        {
            {{-0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, 0.0f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, 0.0f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}}
        };

        std::vector<unsigned int> indices =
        {
            0, 1, 2,
            2, 3, 0
        };

        auto mesh = std::make_shared<Mesh>(vertices, indices);
        mesh->primitive = MeshPrimitive::Plane;
        return mesh;
	}

    std::shared_ptr<Mesh> createCube() {
        std::vector<Vertex> vertices =
        {
            // Front
            {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},

            // Back
            {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f,-1.0f}, {0.0f, 1.0f}},

            // Left
            {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Right
            {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},

            // Top
            {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},

            // Bottom
            {{-0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {0.0f, 0.0f}},
            {{ 0.5f, -0.5f, -0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f, 0.0f}},
            {{ 0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {1.0f, 1.0f}},
            {{-0.5f, -0.5f,  0.5f}, {0.0f,-1.0f, 0.0f}, {0.0f, 1.0f}}
        };

        std::vector<unsigned int> indices =
        {
            0, 1, 2,   2, 3, 0,
            4, 5, 6,   6, 7, 4,    
            8, 9, 10,  10, 11, 8,  
            12, 13, 14, 14, 15, 12,
            16, 17, 18, 18, 19, 16,
            20, 21, 22, 22, 23, 20 
        };

        auto mesh = std::make_shared<Mesh>(vertices, indices);
        mesh->primitive = MeshPrimitive::Box;
        return mesh;
    }
}