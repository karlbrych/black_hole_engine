
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
#define HEIGHT 960
#define WIDTH 1280
// pro mys
double mouseX = 0.0, mouseY = 0.0;
glm::vec2 circleOffset(0.0f, 0.0f);
bool rightPressed = false;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, WIDTH, HEIGHT);
}
void PrintObjects(const Plane &pl) {
    for (size_t i = 0; i < pl.objs.size(); ++i) {
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
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos) {
  mouseX = xpos;
  mouseY = ypos;
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_RIGHT)
    rightPressed = (action == GLFW_PRESS);
}

int main() {

  if (!glfwInit()) {
    std::cerr << "Failed to initialize GLFW\n";
    return -1;
  }

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "OpenGL Circle", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); //remove later
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }
  glEnable(GL_DEPTH_TEST);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
  sphere sphereMesh = createSphere(1.0f, 64, 32);

  Plane plane;

double G = 0.00675;
double dt = 0.005; // your simulation timestep

// --- Central body ---
Object sphere1;
  sphere1.xv = 0;
  sphere1.yv = 0;
  sphere1.zv = -0.0022;
  sphere1.IsBlackHole= false;
  sphere1.radius=2;
  sphere1.VAO = sphereMesh.VAO;
  sphere1.VBO = sphereMesh.VBO;
  sphere1.EBO = sphereMesh.EBO;
  sphere1.pos={12,0,0}; 
  sphere1.indexCount = sphereMesh.indexCount;
    sphere1.modelMatrix =
      glm::translate(glm::mat4(1.0f), sphere1.pos);
	sphere1.mass= 1000;

  Object sphere2;
  sphere2.pos = {15,0,0};
  sphere2.xv = 0;
  sphere2.yv = 0;
  sphere2.zv = 1.5;
  sphere2.IsBlackHole= false;
  sphere2.mass=1;
  sphere2.radius=1;
  sphere2.VAO = sphereMesh.VAO;
  sphere2.VBO = sphereMesh.VBO;
  sphere2.EBO = sphereMesh.EBO;
  sphere2.indexCount = sphereMesh.indexCount;
  sphere2.modelMatrix =
      glm::translate(glm::mat4(1.0f), sphere2.pos);

plane.objs.push_back(&sphere1);
plane.objs.push_back(&sphere2);

  OrthoCamera camera;
  auto lastTime = std::chrono::high_resolution_clock::now();
  int frameCount = 0;
  while (!glfwWindowShouldClose(window)) {
	
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    plane.draw(shader);
    shader.use();
    shader.setMat4("projection", camera.projection);
    shader.setMat4("view", camera.view);
    DoGravity(&plane,G,dt); 
    glfwSwapBuffers(window);
    glfwPollEvents();

    frameCount++;
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = now-lastTime;
    if(elapsed.count() >= 1.0) {
    	std::cout << frameCount << "\n";
	frameCount = 0;
	lastTime=now;
    }

  }
  
  glDeleteVertexArrays(1, &sphereMesh.VAO);
  glDeleteBuffers(1, &sphereMesh.VBO);
  glDeleteBuffers(1, &sphereMesh.EBO);
  glfwTerminate();
  return 0;
}
