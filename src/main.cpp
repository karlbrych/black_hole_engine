
#include "glm/ext/matrix_transform.hpp"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <thread>
#include "models/camera.h"
#include "models/particle.h"
#include "models/shader.h"
#include "models/sphere.h"
#include "models/texture.h"
#include <map>
#define HEIGHT 960
enum class CameraType
{
  Perspective,
  Ortho
};
CameraType currentCamera = CameraType::Perspective;
std::map<int, bool> keysPressed;
float lastMouseX = WIDTH / 2.0f;
float lastMouseY = HEIGHT / 2.0f;
bool firstMouse = true;

float yaw = -90.0f; // start facing -Z
float pitch = 0.0f;
float mouseSensitivity = 0.1f;
#define WIDTH 1280
// pro mys
double mouseX = 0.0, mouseY = 0.0;
glm::vec2 circleOffset(0.0f, 0.0f);
bool rightPressed = false;
static PerspectiveCamera camera;
static glm::vec3 camPos(0.0f, 5.0f, 20.0f);
static glm::vec3 camFront(0.0f, 0.0f, -1.0f);
static glm::vec3 camUp(0.0f, 1.0f, 0.0f);
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
      glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Circle", nullptr, nullptr);
  if (!window)
  {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // remove later
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
  sphere sphereMesh = createSphere(1.0f, 64, 32);
  GLuint texture = Texture::LoadTexture("../src/assets/poop-texture.jpg");
  Plane plane;

  double G = 0.00675;
  double dt = 0.01; // your simulation timestep

  // --- Central body ---
  Object *sphere1 = new Object{
      .pos = {-3, 0, 0},
      .xv = 0,
      .yv = 0,
      .zv = std::sqrt(0.0675 / 12),
      .mass = 10,
      .radius = 1,
      .IsBlackHole = false,
      .VAO = sphereMesh.VAO,
      .VBO = sphereMesh.VBO,
      .EBO = sphereMesh.EBO,
      .indexCount = sphereMesh.indexCount};
  sphere1->modelMatrix = glm::translate(glm::mat4(1.0f), sphere1->pos);

  Object *sphere2 = new Object{
      .pos = {3, 0, 0},
      .xv = 0,
      .yv = 0,
      .zv = -(std::sqrt(0.0675 / 12)),
      .mass = 10,
      .radius = 1,
      .IsBlackHole = false,
      .VAO = sphereMesh.VAO,
      .VBO = sphereMesh.VBO,
      .EBO = sphereMesh.EBO,
      .indexCount = sphereMesh.indexCount,
  };
  sphere2->modelMatrix =
      glm::translate(glm::mat4(1.0f), sphere2->pos);

  plane.objs.push_back(sphere1);
  plane.objs.push_back(sphere2);

  // start camera on the same vertical level as the balls
  camPos.y = 0.0f;
  camFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
  camera.projection =
      glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
  camera.view = glm::lookAt(camPos, camPos + camFront, camUp);

  float lastFrame = 0.0f;
  float DeltaTime = 0.0f;
  static const float moveStep = 7.0; // tune speed
  glfwSetKeyCallback(window, [](GLFWwindow *w, int key, int scancode, int action, int mods)
                     {
    if (action == GLFW_PRESS) keysPressed[key] = true;
    else if (action == GLFW_RELEASE) keysPressed[key] = false; });

  auto lastTime = std::chrono::high_resolution_clock::now();
  int frameCount = 0;
  bool tabPressedLastFrame = false; // mimo hlavní smyčku
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  // main loop
  while (!glfwWindowShouldClose(window))
  {
    // --- Delta time ---
    float currentFrame = glfwGetTime();
    DeltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // --- Zpracování vstupu ---
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    float velocity = moveStep * DeltaTime;
    if (keysPressed[GLFW_KEY_W])
      camPos += camFront * velocity;
    if (keysPressed[GLFW_KEY_S])
      camPos -= camFront * velocity;
    if (keysPressed[GLFW_KEY_A])
      camPos -= right * velocity;
    if (keysPressed[GLFW_KEY_D])
      camPos += right * velocity;
    if (keysPressed[GLFW_KEY_LEFT_SHIFT])
      camPos += camUp * velocity;
    if (keysPressed[GLFW_KEY_LEFT_CONTROL])
      camPos -= camUp * velocity;

    // --- Přepínání kamery ---
    if (keysPressed[GLFW_KEY_TAB] && !tabPressedLastFrame)
    {
      currentCamera = (currentCamera == CameraType::Perspective) ? CameraType::Ortho : CameraType::Perspective;
    }
    tabPressedLastFrame = keysPressed[GLFW_KEY_TAB];

    // --- Aktualizace projekce ---
    if (currentCamera == CameraType::Perspective)
    {
      camera.projection = glm::perspective(glm::radians(45.0f),
                                           (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    }
    else
    {                      // Ortho
      float scale = 10.0f; // velikost scény
      camera.projection = glm::ortho(-scale * ((float)WIDTH / (float)HEIGHT),
                                     scale * ((float)WIDTH / (float)HEIGHT),
                                     -scale, scale,
                                     0.1f, 100.0f);
    }

    camera.view = glm::lookAt(camPos, camPos + camFront, camUp);

    // render
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("projection", camera.projection);
    shader.setMat4("view", camera.view);
    Texture::BindTexture(texture, 0);
    shader.setInt("diffuseTexture", 0);
    plane.draw(shader);
    DoGravity(&plane, G, dt);
    glfwSwapBuffers(window);
    glfwPollEvents();

    frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now - lastTime;
    if (elapsed.count() >= 1.0)
    {
      std::cout << frameCount << "\n";
      frameCount = 0;
      lastTime = now;
    }
  }

  glDeleteVertexArrays(1, &sphereMesh.VAO);
  glDeleteBuffers(1, &sphereMesh.VBO);
  glDeleteBuffers(1, &sphereMesh.EBO);
  glfwTerminate();
  std::cout << "Process terminated\n";
  return 0;
}
