#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
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
float lastMouseX,lastMouseY;
// pro mys
double mouseX = 0.0, mouseY = 0.0;
glm::vec2 circleOffset(0.0f, 0.0f);
bool rightPressed = false;


static glm::vec3 camPos(0.0f, 5.0f, 20.0f);
static glm::vec3 camFront(0.0f, 0.0f, -1.0f);
static glm::vec3 camUp(0.0f, 1.0f, 0.0f);
float cameraSpeed = 7.0f;

float deltaTime = 0.0f;
float lastFrame;
bool gameStopped = false;
bool tabPressedLastFrame = false;
bool escapePressedLastFrame = false;
void processInput(GLFWwindow *window)
{
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));
    float velocity = cameraSpeed * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS && !escapePressedLastFrame) {
	if(gameStopped) {
	    	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);	
	}
	else {
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
  GLFWmonitor* monitor = glfwGetPrimaryMonitor();
  if (!monitor) {
    std::cerr << "No monitor detected!\n";
    return -1;
  }
  const GLFWvidmode* mode = glfwGetVideoMode(monitor);
  WIDTH= mode->width;
  HEIGHT = mode->height;
  lastMouseX = WIDTH / 2.0f;
  lastMouseY = HEIGHT / 2.0f;
  std::cout<< WIDTH << " " << HEIGHT << "\n";
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  shader shader("shaders/vertex.glsl", "shaders/fragment.glsl");
  sphere sphereMesh = createSphere(1.0f, 64, 32);
  GLuint texture1 = Texture::LoadTexture("assets/planet.jpg");
  GLuint texture2 = Texture::LoadTexture("assets/poop-texture.jpg");
  GLuint texture3 = Texture::LoadTexture("assets/sun-texture.jpg");
  Plane plane;

  double G = 0.00675;
  double dt = 0.01; // your simulation timestep

  // --- Central body ---
  Object *sphere1 = new Object{
      .pos = {-13, 0, 0},
      .xv = 0,
      .yv = 0,
      .zv = std::sqrt(0.0675 / 12),
      .mass = 1,
      .radius = 1,
      .IsBlackHole = false,
      .VAO = sphereMesh.VAO,
      .VBO = sphereMesh.VBO,
      .EBO = sphereMesh.EBO,
      .indexCount = sphereMesh.indexCount,
      .textureId = texture1
    };
    
  sphere1->modelMatrix = glm::translate(glm::mat4(1.0f), sphere1->pos);

  Object *sphere2 = new Object{
      .pos = {3, 0, 0},
      .xv = 0,
      .yv = 0,
      .zv = -(std::sqrt(0.0675 / 12)),
      .mass = 10,
      .radius = 3,
      .IsBlackHole = false,
      .VAO = sphereMesh.VAO,
      .VBO = sphereMesh.VBO,
      .EBO = sphereMesh.EBO,
      .indexCount = sphereMesh.indexCount,
      .textureId = texture3
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


  auto lastTime = std::chrono::high_resolution_clock::now();
  int frameCount = 0;
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

  // main loop
  while (!glfwWindowShouldClose(window))
  {
    processInput(window);
    escapePressedLastFrame = (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS);
    tabPressedLastFrame = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    if(!gameStopped) {
float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;



    // --- Zpracování vstupu ---
	  //std::cout << tabPressedLastFrame << std::endl;
    // --- Aktualizace projekce ---
    
//změna kamery mezi ortho a persective
      float scale = 10.0f; // velikost scény
    camera.projection = (currentCamera == CameraType::Perspective) ? (glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f)) : (glm::ortho(-scale * ((float)WIDTH / (float)HEIGHT), scale * ((float)WIDTH / (float)HEIGHT), -scale, scale, 0.1f, 100.0f));
    

    camera.view = glm::lookAt(camPos, camPos + camFront, camUp);

    // render

    glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.use();
    shader.setMat4("projection", camera.projection);
    shader.setMat4("view", camera.view);
    shader.setInt("diffuseTexture", 0);
    float tmp = glfwGetTime();
    plane.rotate(tmp);
    plane.draw(shader);
    DoGravity(&plane, G, dt);
    }
    else {
    glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ImGui::SetNextWindowPos(ImVec2(WIDTH / 2.0f - 200, HEIGHT / 2.0f - 100), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(400, 200), ImGuiCond_Once);

    // Set window flags to remove the background
    ImGui::Begin("Main Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground);

    // Large text in the center of the window
    ImGui::Text("Main Menu");
    // Buttons in the middle
    if (ImGui::Button("Start Game", ImVec2(200, 50))) {
        gameStopped = false;
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    ImGui::SameLine();
    if (ImGui::Button("Exit", ImVec2(200, 50))) {
        glfwSetWindowShouldClose(window, GLFW_TRUE); // Exit game
    }

    ImGui::End();

      }
            ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
    glfwPollEvents();


  }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

  glDeleteVertexArrays(1, &sphereMesh.VAO);
  glDeleteBuffers(1, &sphereMesh.VBO);
  glDeleteBuffers(1, &sphereMesh.EBO);
  glfwTerminate();
  std::cout << "Process terminated\n";
  return 0;
}
