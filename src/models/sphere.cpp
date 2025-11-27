#include "sphere.h"
#include "glm/glm.hpp"
#include <vector>
#include <cmath>
#include <glad/glad.h>

sphere createSphere(float radius, unsigned int sectors, unsigned int stacks)
{
    std::vector<float> vertices;
    std::vector<unsigned int> indices;

    float x, y, z, xy;
    float sectorStep = 2.0f * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    // Generate vertices
    for (unsigned int i = 0; i <= stacks; ++i)
    {
        stackAngle = M_PI / 2.0f - i * stackStep; // from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (unsigned int j = 0; j <= sectors; ++j)
        {
            sectorAngle = j * sectorStep;

            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            // Positions
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

           

            float u = ((float)j / sectors) ;
            float v = ((float)i / stacks);

            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    // Generate indices
    for (unsigned int i = 0; i < stacks; ++i)
    {
        unsigned int k1 = i * (sectors + 1); // beginning of current stack
        unsigned int k2 = k1 + sectors + 1;  // beginning of next stack

        for (unsigned int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
                indices.insert(indices.end(), {k1, k2, k1 + 1});
            if (i != (stacks - 1))
                indices.insert(indices.end(), {k1 + 1, k2, k2 + 1});
        }
    }

    sphere mesh;

    // Generate OpenGL buffers
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Vertex attributes
    // Position (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Texture coordinates (location = 1)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    mesh.indexCount = indices.size();
    return mesh;
}
