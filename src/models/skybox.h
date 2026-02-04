#include <vector>
#include <string>
#include <glad/glad.h>
#include "shader.h"
#include <glm/glm.hpp>
class skybox {
public:
    skybox();
    ~skybox();
    void loadTextures(const std::vector<std::string>& faces);
    void render(const glm::mat4& view, const glm::mat4& projection, float time);
    void setRotationSpeed(float speed);
private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    shader skyboxShader;
    float rotationSpeed;
};
