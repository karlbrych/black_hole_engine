#pragma once
#include "core.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "sphere.h"
#include "glm/vec3.hpp"
class Object_Editor{
public:
    Object_Editor();
    ~Object_Editor();

    void init(GLFWwindow* window, const char* glsl_version);
    void shutdown();
    void beginFrame();
    bool render(int windowWidth, int windowHeight, Plane* plane,
                const sphere& mesh,
                GLuint planetTexture,
                GLuint poopTexture,
                GLuint sunTexture,
                glm::vec3* cameraPosition,
                bool* firstMouse);
    void drawPreview(const class shader& objectShader, glm::mat4 projection, glm::mat4 view) const;

    bool isOpen() const { return open_; }
    void toggleOpen();

private:
    glm::vec3 resolveSpawnPosition(const Plane* plane) const;
    void syncPreview(const sphere& mesh, Plane* plane, GLuint planetTexture, GLuint poopTexture, GLuint sunTexture);
    void clearPreview();
    GLuint resolveTexture(GLuint planetTexture, GLuint poopTexture, GLuint sunTexture) const;
    GLuint createPreviewTexture() const;

    bool open_ = false;
    float position_[3] = {0.0f, 0.0f, 0.0f};
    float velocity_[3] = {0.0f, 0.0f, 0.0f};
    float mass_ = 1.0f;
    float radius_ = 1.0f;
    int textureChoice_ = 0;
    bool isBlackHole_ = false;
    bool isLightSource_ = false;
    glm::vec3 cameraPos_= glm::vec3(0.0f);
    bool previewVisible_ = false;
    GLuint previewTexture_ = 0;
    Object previewObject_;
};