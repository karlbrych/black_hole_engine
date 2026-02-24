#include "glm/ext/matrix_transform.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdio>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <thread>
#include "models/particle.h"
#include "models/shader.h"
#include "models/sphere.h"
#include "models/texture.h"
#include <map>
#include "models/camera.h"
#include "models/skybox.h"
#include "models/startmenu.h"
enum class CameraType
{
  Perspective,
  Ortho
};
// CAMERA
CameraType currentCamera = CameraType::Perspective;
static PerspectiveCamera camera;
std::map<int, bool> keysPressed;
bool firstMouse = true;

float yaw = -90.0f; // start facing -Z
float pitch = 0.0f;
float mouseSensitivity = 0.1f;
float lastMouseX, lastMouseY;
// pro mys
double mouseX = 0.0, mouseY = 0.0;
glm::vec2 circleOffset(0.0f, 0.0f);
bool rightPressed = false;

static glm::vec3 camPos(0.0f, 5.0f, 20.0f);
static glm::vec3 camFront(0.0f, 0.0f, -1.0f);
static glm::vec3 camUp(0.0f, 1.0f, 0.0f);
float cameraSpeed = 100000.0f;

float deltaTime = 0.0f;
float lastFrame;
bool gameStopped = false;
bool tabPressedLastFrame = false;
bool escapePressedLastFrame = false;
void processInput(GLFWwindow *window)
{
  glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
  float velocity = cameraSpeed * deltaTime;
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escapePressedLastFrame)
  {
    if (gameStopped)
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else
    {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    gameStopped = !gameStopped;
    
  }
  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camPos += camFront * velocity;
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camPos -= camFront * velocity;
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camPos -= right * velocity;
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camPos += right * velocity;
  if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    camPos += camUp * velocity;
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    camPos -= camUp * velocity;
  currentCamera = ((glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) && !tabPressedLastFrame) ? ((currentCamera == CameraType::Perspective) ? CameraType::Ortho : CameraType::Perspective) : currentCamera;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
  glViewport(0, 0, WIDTH, HEIGHT);
}

void PrintObjects(const Plane &pl)
{
  for (size_t i = 0; i < pl.objs.size(); ++i)
  {
    const Object &o = *pl.objs[i];
    printf("Object %zu:\n", i + 1);
    printf("  Position: (%.2f, %.2f, %.2f)\n", o.pos.x, o.pos.y, o.pos.z);
    printf("  IsBlackHole: %d\n", o.IsBlackHole);
    printf("  Mass: %.2f\n", o.mass);
    printf("  Radius: %.2f\n", o.radius);
    printf("  Velocity: (%.10f, %.10f, %.10f)\n", o.xv, o.yv, o.zv);
    printf("--------------------------\n");
  }
}
// Mouse callbacks
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastMouseX = xpos;
    lastMouseY = ypos;
    firstMouse = false;
  }

  if(gameStopped){
    firstMouse = true;
    return;
  }
  
  float xoffset = xpos - lastMouseX;
  float yoffset = lastMouseY - ypos; // reversed Y
  lastMouseX = xpos;
  lastMouseY = ypos;

  xoffset *= mouseSensitivity;
  yoffset *= mouseSensitivity;

  yaw += xoffset;
  pitch += yoffset;

  // clamp pitch
  if (pitch > 89.0f)
    pitch = 89.0f;
  if (pitch < -89.0f)
    pitch = -89.0f;

  // recalc camera front vector
  glm::vec3 front;
  front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
  front.y = sin(glm::radians(pitch));
  front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

  camFront = glm::normalize(front);
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods)
{
  if (button == GLFW_MOUSE_BUTTON_RIGHT)
    rightPressed = (action == GLFW_PRESS);
}

int main()
{

  if (!glfwInit())
  {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "black_hole_engine:OpenGL", nullptr, nullptr);
  if (!window)
  {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // remove later
  GLFWmonitor *monitor = glfwGetPrimaryMonitor();
  if (!monitor)
  {
    std::cerr << "No monitor detected!\n";
    return -1;
  }
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  WIDTH = mode->width;
  HEIGHT = mode->height;
  lastMouseX = WIDTH / 2.0f;
  lastMouseY = HEIGHT / 2.0f;
  std::cout << WIDTH << " " << HEIGHT << "\n";
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  shader Shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  shader LightSourceShader("shaders/vertex.glsl", "shaders/light_source_fragment.glsl");
  shader skyboxShader("shaders/skybox_vertex.glsl", "shaders/skybox_fragment.glsl");
  sphere sphereMesh = createSphere(1.0f, 64, 32);
  GLuint texture1 = Texture::LoadTexture("assets/planet.jpg");
  GLuint texture2 = Texture::LoadTexture("assets/poop-texture.jpg");
  GLuint texture3 = Texture::LoadTexture("assets/sun-texture.jpg");

  Plane plane;
  skybox skybox;
  skybox.loadTextures({// loading skybox textures
                       "assets/skybox/right.png",
                       "assets/skybox/left.png",
                       "assets/skybox/top.png",
                       "assets/skybox/bottom.png",
                       "assets/skybox/front.png",
                       "assets/skybox/back.png"});
bool scaleSystem = true;
double SCALE = scaleSystem ? 100000 : 1.0;
double MASS_SCALE = SCALE * SCALE * SCALE;
plane.G = 4.0 * M_PI * M_PI;
plane.dt = 0.0001;

// Sun
Object* sun = new Object(
    0, 0, 0,
    glm::vec3(0.0, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    1.0 * MASS_SCALE,
    sphereMesh.indexCount,
    0.25f * SCALE,
    texture3,
    false
);

// Mercury
Object* mercury = new Object(
    0, 0, std::sqrt(plane.G / 0.387) * SCALE,
    glm::vec3(0.387 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    1.65e-7 * MASS_SCALE,
    sphereMesh.indexCount,
    0.01f * SCALE,
    texture1,
    false
);

// Venus
Object* venus = new Object(
    0, 0, std::sqrt(plane.G / 0.723) * SCALE,
    glm::vec3(0.723 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    2.45e-6 * MASS_SCALE,
    sphereMesh.indexCount,
    0.02f * SCALE,
    texture1,
    false,
	true
);

// Earth
Object* earth = new Object(
    0, 0, std::sqrt(plane.G / 1.0) * SCALE,
    glm::vec3(1.0 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    3.00e-6 * MASS_SCALE,
    sphereMesh.indexCount,
    0.02f * SCALE,
    texture1,
    false
);

// Mars
Object* mars = new Object(
    0, 0, std::sqrt(plane.G / 1.524) * SCALE,
    glm::vec3(1.524 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    3.23e-7 * MASS_SCALE,
    sphereMesh.indexCount,
    0.015f * SCALE,
    texture1,
    false
);

// Jupiter
Object* jupiter = new Object(
    0, 0, std::sqrt(plane.G / 5.203) * SCALE,
    glm::vec3(5.203 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    9.54e-4 * MASS_SCALE,
    sphereMesh.indexCount,
    0.08f * SCALE,
    texture1,
    false
);

// Saturn
Object* saturn = new Object(
    0, 0, std::sqrt(plane.G / 9.537) * SCALE,
    glm::vec3(9.537 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    2.86e-4 * MASS_SCALE,
    sphereMesh.indexCount,
    0.07f * SCALE,
    texture1,
    false
);

// Uranus
Object* uranus = new Object(
    0, 0, std::sqrt(plane.G / 19.191) * SCALE,
    glm::vec3(19.191 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    4.37e-5 * MASS_SCALE,
    sphereMesh.indexCount,
    0.05f * SCALE,
    texture1,
    false
);

// Neptune
Object* neptune = new Object(
    0, 0, std::sqrt(plane.G / 30.07) * SCALE,
    glm::vec3(30.07 * SCALE, 0.0, 0.0),
    sphereMesh.VAO,
    sphereMesh.VBO,
    sphereMesh.EBO,
    5.15e-5 * MASS_SCALE,
    sphereMesh.indexCount,
    0.05f * SCALE,
    texture1,
    false
);

// Push into container
plane.objs.push_back(sun);
plane.objs.push_back(mercury);
plane.objs.push_back(venus);
plane.objs.push_back(earth);
plane.objs.push_back(mars);
plane.objs.push_back(jupiter);
plane.objs.push_back(saturn);
plane.objs.push_back(uranus);
plane.objs.push_back(neptune);



  camPos.y = 0.0f;
  camFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
  camera.projection =
      glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
  camera.view = glm::lookAt(camPos, camPos + camFront, camUp);

  auto lastTime = std::chrono::high_resolution_clock::now();
  int frameCount = 0;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  
  // Initialize start menu
  StartMenu startMenu;
  startMenu.init(window, "#version 330");



  // main loop
  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    escapePressedLastFrame = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
    tabPressedLastFrame = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
    
    startMenu.beginFrame();
    
    // Hide/show cursor based on game state
    if (!gameStopped) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } 
    else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    
    if (!gameStopped)
    {
      float currentFrame = glfwGetTime();
      deltaTime = currentFrame - lastFrame;
      lastFrame = currentFrame;

      // --- Zpracování vstupu ---
      // std::cout << tabPressedLastFrame << std::endl;
      // --- Aktualizace projekce ---
      if (currentCamera == CameraType::Perspective)
      {
        camera.projection = glm::perspective(glm::radians(45.0f),
                                             (float)WIDTH / (float)HEIGHT, 0.1f, 100000000.0f);
      }
      else
      {                      // Ortho
        float scale = 10.0f; // velikost scény
        camera.projection = glm::ortho(-scale * ((float)WIDTH / (float)HEIGHT),
                                       scale * ((float)WIDTH / (float)HEIGHT),
                                       -scale, scale,
                                       0.1f, 100000000.0f);
      }

      camera.view = glm::lookAt(camPos, camPos + camFront, camUp);

      // render

      glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      float tmp = glfwGetTime();

      // Render skybox first with slow rotation
      skybox.setRotationSpeed(0.13f); // degrees per second
      skybox.render(camera.view, camera.projection, tmp);

      plane.rotate(tmp);
      plane.draw(Shader, LightSourceShader, camera.projection, camera.view);
      DoGravity(&plane, plane.G, plane.dt);
    }
    else
    {
      glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      
      startMenu.render(WIDTH, HEIGHT, &plane);
      
      if (startMenu.shouldStartGame())
      {
        gameStopped = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        startMenu.reset();
      }
      
      if (startMenu.shouldExitGame())
      {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
      }
    }
    
    startMenu.endFrame();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  
  startMenu.shutdown();

  glDeleteVertexArrays(1, &sphereMesh.VAO);
  glDeleteBuffers(1, &sphereMesh.VBO);
  glDeleteBuffers(1, &sphereMesh.EBO);
  glfwTerminate();
  std::cout << "Process terminated\n";
  return 0;
}
