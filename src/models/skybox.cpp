#include "skybox.h"
#include "stb_image.h"
#include <iostream>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Skybox vertices - a cube centered at origin
static float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

skybox::skybox() 
    : skyboxVAO(0), skyboxVBO(0), cubemapTexture(0),
      skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl"),
      rotationSpeed(10.0f)
{
    // Generate and setup VAO/VBO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    glBindVertexArray(0);
}

skybox::~skybox()
{
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    if (cubemapTexture != 0) {
        glDeleteTextures(1, &cubemapTexture);
    }
}

void skybox::loadTextures(const std::vector<std::string>& faces)
{
    if (faces.size() != 6) {
        std::cerr << "ERROR: Skybox requires exactly 6 texture faces" << std::endl;
        return;
    }

    glGenTextures(1, &cubemapTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(false); // Don't flip cubemap textures
    
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            std::cout << "Loaded skybox texture: " << faces[i] << " (" << width << "x" << height << ")" << std::endl;
            stbi_image_free(data);
        }
        else
        {
            std::cerr << "ERROR: Failed to load cubemap texture: " << faces[i] << std::endl;
            std::cerr << "Reason: " << stbi_failure_reason() << std::endl;
        }
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void skybox::render(const glm::mat4& view, const glm::mat4& projection, float time)
{
    glDepthFunc(GL_LEQUAL);
    
    skyboxShader.use();
    
    // Remove translation from view matrix
    glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
    
    // Apply rotation to the skybox based on time
    float rotationAngle = time * rotationSpeed;
    glm::mat4 rotatedView = glm::rotate(viewNoTranslation, glm::radians(rotationAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    
    skyboxShader.setMat4("view", rotatedView);
    skyboxShader.setMat4("projection", projection);
    
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    skyboxShader.setInt("skybox", 0);
    
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

void skybox::setRotationSpeed(float speed)
{
    rotationSpeed = speed;
}
