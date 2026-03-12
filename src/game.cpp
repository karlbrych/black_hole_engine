#define _USE_MATH_DEFINES
#include "game.h"
#include <cmath>
#include <iostream>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include "models/texture.h"

Game* Game::instance_ = nullptr;

void Game::printObjects(const Plane& plane)
{
    std::cout << "Plane info:\n";
    std::cout << "G: " << plane.G << "\n";
    std::cout << "dt: " << plane.dt << "\n";
    std::cout << "Object count: " << plane.objs.size() << "\n\n";

    for (size_t i = 0; i < plane.objs.size(); i++)
    {
        const Object* obj = plane.objs[i];

        std::cout << "Object #" << i << "\n";
        std::cout << "Position: "
                  << obj->pos.x << ", "
                  << obj->pos.y << ", "
                  << obj->pos.z << "\n";

        std::cout << "Velocity: "
                  << obj->xv << ", "
                  << obj->yv << ", "
                  << obj->zv << "\n";

        std::cout << "Mass: " << obj->mass << "\n";
        std::cout << "Radius: " << obj->radius << "\n";

        std::cout << "IsBlackHole: " << obj->IsBlackHole << "\n";
        std::cout << "IsLightSource: " << obj->IsLightSource << "\n";

        std::cout << "TextureID: " << obj->textureId << "\n";
        std::cout << "IndexCount: " << obj->indexCount << "\n";

        std::cout << "VAO: " << obj->VAO
                  << " VBO: " << obj->VBO
                  << " EBO: " << obj->EBO << "\n";

        std::cout << "-----------------------------\n";
    }
}

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
    texture1_ = Texture::LoadTexture("assets/planet.png");
    texture2_ = Texture::LoadTexture("assets/poop-texture.jpg");
    texture3_ = Texture::LoadTexture("assets/sun-texture.jpg");

    skybox_->loadPreprocessedTextures({
        "assets/skybox/right.png",
        "assets/skybox/left.png",
        "assets/skybox/top.png",
        "assets/skybox/bottom.png",
        "assets/skybox/front.png",
        "assets/skybox/back.png"
    });

    plane_.G = 0.00675;
    plane_.dt = 0.0001;

    setupSolarSystem();

    camFront_ = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    startMenu_.init(window_, "#version 330");
    camera_.projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.001f, static_cast<float>(scale_) * 5000.0f);
}

void Game::setupSolarSystem()
{	
std::mt19937 rng(42);

  std::uniform_real_distribution<float> posDist(-100.0f, 100.0f);
  std::uniform_real_distribution<float> velDist(-5.0f, 5.0f);
  std::uniform_real_distribution<float> massDist(900000.0f, 1000000.0f);   // <- integer distribution
  std::uniform_real_distribution<float> radiusDist(0.5f, 5.0f);

for (int i = 0; i < 500; i++)
{
    Object* obj = new Object(
        velDist(rng),
        velDist(rng),
        velDist(rng),
        glm::vec3(
            posDist(rng),
            posDist(rng),
            posDist(rng)
        ),
        sphereMesh_.VAO,
        sphereMesh_.VBO,
        sphereMesh_.EBO,
        massDist(rng),
        sphereMesh_.indexCount,
        radiusDist(rng),
        texture1_,
        false
    );

    plane_.objs.push_back(obj);
}
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

    plane_.rotate(time);
    plane_.draw(*objectShader_, *lightSourceShader_, camera_.projection, camera_.view);
    DoGravity(&plane_, plane_.G, plane_.dt);
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
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        } else {
            glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }

        if (!gameStopped_) {
            renderRunningFrame();
        } else {
            renderMenuFrame();
        }
	//printObjects(plane_);
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
    if (texture1_ != 0) {
        glDeleteBuffers(1, &texture1_);
    }
    if (texture2_ != 0) {
        glDeleteBuffers(1, &texture2_);
    }
    if (texture3_ != 0) {
        glDeleteBuffers(1, &texture3_);
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
