#define _USE_MATH_DEFINES
#include "game.h"
#include <cmath>
#include <iostream>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "models/texture.h"
#include "imgui.h"

Game* Game::instance_ = nullptr;

void Game::framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    (void)window;
    (void)width;
    (void)height;
    glViewport(0, 0, WIDTH, HEIGHT);
}

void Game::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    (void)window;
    if (instance_ != nullptr) {
        instance_->updateCameraFromMouse(xpos, ypos);
    }
}

void Game::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    (void)window;
    (void)mods;
    if (instance_ == nullptr) {
        return;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        instance_->rightPressed_ = (action == GLFW_PRESS);
    }
}

bool Game::initialize()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
    window_ = glfwCreateWindow(WIDTH, HEIGHT, "black_hole_engine:OpenGL", nullptr, nullptr);
    if (!window_) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(0);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    if (!monitor) {
        std::cerr << "No monitor detected!\n";
        return false;
    }

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    WIDTH = mode->width;
    HEIGHT = mode->height;
    lastMouseX_ = WIDTH / 2.0f;
    lastMouseY_ = HEIGHT / 2.0f;

    std::cout << WIDTH << " " << HEIGHT << "\n";

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    instance_ = this;
    glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
    glfwSetCursorPosCallback(window_, cursorPositionCallback);
    glfwSetMouseButtonCallback(window_, mouseButtonCallback);

    initializeScene();
    return true;
}

void Game::initializeScene()
    {
    skybox_ = std::make_unique<skybox>();

    objectShader_ = std::make_unique<shader>("shaders/vertex.glsl", "shaders/fragment.glsl");
    lightSourceShader_ = std::make_unique<shader>("shaders/vertex.glsl", "shaders/light_source_fragment.glsl");

    sphereMesh_ = createSphere(1.0f, 64, 32);
    textures.push_back(Texture::LoadTexture("assets/planet.png"));
    textures.push_back(Texture::LoadTexture("assets/poop-texture.jpg"));
    textures.push_back(Texture::LoadTexture("assets/sun-texture.jpg"));

    skybox_->loadPreprocessedTextures({
        "assets/skybox/right.png",
        "assets/skybox/left.png",
        "assets/skybox/top.png",
        "assets/skybox/bottom.png",
        "assets/skybox/front.png",
        "assets/skybox/back.png"
    });

    const bool scaleSystem = true;
    scale_ = scaleSystem ? 100000.0 : 1.0;
    plane_.G = 4.0 * M_PI * M_PI;
    plane_.dt = 0.0001;

    //setupSolarSystem();

    camFront_ = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    startMenu_.init(window_, "#version 330");
    camera_.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 10.0f, static_cast<float>(scale_) * 50.0f);
}

void Game::setupSolarSystem()
{
    const double massScale = scale_ * scale_ * scale_;

    Object* sun = new Object(
        0, 0, 0,
        glm::vec3(0.0, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        1.0 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.25 * scale_),
        textures[2],
        false,
        true
    );

    Object* mercury = new Object(
        0, 0, std::sqrt(plane_.G / 0.387) * scale_,
        glm::vec3(0.387 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        1.65e-7 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.01 * scale_),
        textures[0],
        false
    );

    Object* venus = new Object(
        0, 0, std::sqrt(plane_.G / 0.723) * scale_,
        glm::vec3(0.723 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        2.45e-6 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.02 * scale_),
        textures[0],
        false
    );

    Object* earth = new Object(
        0, 0, std::sqrt(plane_.G / 1.0) * scale_,
        glm::vec3(1.0 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        3.00e-6 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.02 * scale_),
        textures[0],
        false
    );

    Object* mars = new Object(
        0, 0, std::sqrt(plane_.G / 1.524) * scale_,
        glm::vec3(1.524 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        3.23e-7 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.015 * scale_),
        textures[0],
        false
    );

    Object* jupiter = new Object(
        0, 0, std::sqrt(plane_.G / 5.203) * scale_,
        glm::vec3(5.203 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        9.54e-4 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.08 * scale_),
        textures[0],
        false
    );

    Object* saturn = new Object(
        0, 0, std::sqrt(plane_.G / 9.537) * scale_,
        glm::vec3(9.537 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        2.86e-4 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.07 * scale_),
        textures[0],
        false
    );

    Object* uranus = new Object(
        0, 0, std::sqrt(plane_.G / 19.191) * scale_,
        glm::vec3(19.191 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        4.37e-5 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.05 * scale_),
        textures[0],
        false
    );

    Object* neptune = new Object(
        0, 0, std::sqrt(plane_.G / 30.07) * scale_,
        glm::vec3(30.07 * scale_, 0.0, 0.0),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        5.15e-5 * massScale,
        sphereMesh_.indexCount,
        static_cast<float>(0.05 * scale_),
        textures[0],
        false
    );

    plane_.objs.push_back(sun);
    plane_.objs.push_back(mercury);
    plane_.objs.push_back(venus);
    plane_.objs.push_back(earth);
    plane_.objs.push_back(mars);
    plane_.objs.push_back(jupiter);
    plane_.objs.push_back(saturn);
    plane_.objs.push_back(uranus);
    plane_.objs.push_back(neptune);
}

void Game::processInput()
{
    glm::vec3 right = glm::normalize(glm::cross(camFront_, camUp_));
    float velocity = cameraSpeed_ * deltaTime_;

    if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escapePressedLastFrame_) {
        if (gameStopped_) {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        } else {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        gameStopped_ = !gameStopped_;
    }

    if (!gameStopped_ && glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS && !tabPressedLastFrame_) {
        if (!objectEditor_.isOpen()) {
            double cursorX = 0.0;
            double cursorY = 0.0;
            glfwGetCursorPos(window_, &cursorX, &cursorY);
            editorMouseX_ = static_cast<float>(cursorX);
            editorMouseY_ = static_cast<float>(cursorY);
            objectEditor_.toggleOpen();
            firstMouse_ = true;
        } else {
            objectEditor_.toggleOpen();
            firstMouse_ = true;
            glfwSetCursorPos(window_, static_cast<double>(editorMouseX_), static_cast<double>(editorMouseY_));
        }
    }
    if (!gameStopped_) {
        glfwSetInputMode(window_, GLFW_CURSOR, objectEditor_.isOpen() ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        glfwSetInputMode(window_, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
    if (gameStopped_) {
        return;
    }

    if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
        camPos_ += camFront_ * velocity;
    }
    if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
        camPos_ -= camFront_ * velocity;
    }
    if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
        camPos_ -= right * velocity;
    }
    if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
        camPos_ += right * velocity;
    }
    if (glfwGetKey(window_, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        camPos_ += camUp_ * velocity;
    }
    if (glfwGetKey(window_, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        camPos_ -= camUp_ * velocity;
    }
}

void Game::updateCameraFromMouse(double xpos, double ypos)
{
    if (objectEditor_.isOpen()) {
        return;
    }

    if (firstMouse_) {
        lastMouseX_ = static_cast<float>(xpos);
        lastMouseY_ = static_cast<float>(ypos);
        firstMouse_ = false;
    }

    if (gameStopped_) {
        firstMouse_ = true;
        return;
    }

    float xoffset = static_cast<float>(xpos) - lastMouseX_;
    float yoffset = lastMouseY_ - static_cast<float>(ypos);
    lastMouseX_ = static_cast<float>(xpos);
    lastMouseY_ = static_cast<float>(ypos);

    xoffset *= mouseSensitivity_;
    yoffset *= mouseSensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (pitch_ > 89.0f) {
        pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f) {
        pitch_ = -89.0f;
    }

    glm::vec3 front;
    front.x = std::cos(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    front.y = std::sin(glm::radians(pitch_));
    front.z = std::sin(glm::radians(yaw_)) * std::cos(glm::radians(pitch_));
    camFront_ = glm::normalize(front);
}

void Game::renderRunningFrame()
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime_ = currentFrame - lastFrame_;
    lastFrame_ = currentFrame;

    camera_.view = glm::lookAt(camPos_, camPos_ + camFront_, camUp_);

    glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = static_cast<float>(glfwGetTime());

    if (skybox_ != nullptr) {
        skybox_->setRotationSpeed(0.13f);
        skybox_->render(camera_.view, camera_.projection, time);
    }

    if (!objectEditor_.isOpen()) {
        plane_.rotate(time);
    }
    plane_.draw(*objectShader_, *lightSourceShader_, camera_.projection, camera_.view,camPos_);
    if (!objectEditor_.isOpen()) {
        DoGravity(&plane_, plane_.G, plane_.dt);
    }

    if (objectEditor_.isOpen()) {
        objectEditor_.render(WIDTH+10, HEIGHT+10, &plane_, sphereMesh_, textures[0], textures[1], textures[2], &camPos_, &firstMouse_);
        objectEditor_.drawPreview(*objectShader_, camera_.projection, camera_.view);
    }
}

void Game::renderMenuFrame()
{
    glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    startMenu_.render(WIDTH, HEIGHT, &plane_);

    if (startMenu_.shouldStartGame()) {
        gameStopped_ = false;
        glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        startMenu_.reset();
    }

    if (startMenu_.shouldExitGame()) {
        glfwSetWindowShouldClose(window_, GLFW_TRUE);
    }
}

int Game::run()
{
    if (!initialize()) {
        cleanup();
        return -1;
    }

    while (!glfwWindowShouldClose(window_)) {
        processInput();
        escapePressedLastFrame_ = (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS);
        tabPressedLastFrame_ = (glfwGetKey(window_, GLFW_KEY_TAB) == GLFW_PRESS);

        startMenu_.beginFrame();

        if (!gameStopped_) {
            glfwSetInputMode(window_, GLFW_CURSOR, objectEditor_.isOpen() ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (!gameStopped_) {
            renderRunningFrame();
        } else {
            renderMenuFrame();
        }

        startMenu_.endFrame();
        glfwSwapBuffers(window_);
        glfwPollEvents();
    }

    cleanup();
    std::cout << "Process terminated\n";
    return 0;
}

void Game::cleanup()
{
    if (window_ != nullptr) {
        startMenu_.shutdown();
    }

    if (sphereMesh_.VAO != 0) {
        glDeleteVertexArrays(1, &sphereMesh_.VAO);
    }
    if (sphereMesh_.VBO != 0) {
        glDeleteBuffers(1, &sphereMesh_.VBO);
    }
    if (sphereMesh_.EBO != 0) {
        glDeleteBuffers(1, &sphereMesh_.EBO);
    }
    if (!textures.empty()) {
        glDeleteBuffers(1, &textures[0]);
    }

    for (Object* obj : plane_.objs) {
        delete obj;
    }
    plane_.objs.clear();

    skybox_.reset();

    if (window_ != nullptr || glfwGetCurrentContext() != nullptr) {
        glfwTerminate();
    }

    window_ = nullptr;
    instance_ = nullptr;
}
