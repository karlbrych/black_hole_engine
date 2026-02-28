#pragma once

#include <memory>

#include "models/camera.h"
#include "models/core.h"
#include "models/shader.h"
#include "models/skybox.h"
#include "models/sphere.h"
#include "models/startmenu.h"

struct GLFWwindow;

class Game {
public:
    int run();

private:
    enum class CameraType {
        Perspective
    };

    bool initialize();
    void initializeScene();
    void setupSolarSystem();
    void processInput();
    void updateCameraFromMouse(double xpos, double ypos);
    void renderRunningFrame();
    void renderMenuFrame();
    void cleanup();

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

    GLFWwindow* window_ = nullptr;

    CameraType currentCamera_ = CameraType::Perspective;
    PerspectiveCamera camera_;

    bool firstMouse_ = true;
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float mouseSensitivity_ = 0.1f;
    float lastMouseX_ = 0.0f;
    float lastMouseY_ = 0.0f;

    bool rightPressed_ = false;

    glm::vec3 camPos_ = glm::vec3(300000.0f, 0.0f, 0.0f);
    glm::vec3 camFront_ = glm::vec3(1.0f, 0.0f, -1.0f);
    glm::vec3 camUp_ = glm::vec3(0.0f, 1.0f, 0.0f);

    float cameraSpeed_ = 1000.0f;
    float deltaTime_ = 0.0f;
    float lastFrame_ = 0.0f;

    bool gameStopped_ = true;
    bool tabPressedLastFrame_ = false;
    bool escapePressedLastFrame_ = false;

    double scale_ = 1.0;

    Plane plane_;
    std::unique_ptr<skybox> skybox_;
    sphere sphereMesh_{};
    StartMenu startMenu_;

    std::unique_ptr<shader> objectShader_;
    std::unique_ptr<shader> lightSourceShader_;

    GLuint texture1_ = 0;
    GLuint texture2_ = 0;
    GLuint texture3_ = 0;

    static Game* instance_;
};
