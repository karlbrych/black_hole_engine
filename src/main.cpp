#include "glm/ext/matrix_transform.hpp"
#include "models/camera.h"
#include "models/particle.h"
#include "models/shader.h"
#include "models/sphere.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#define HEIGHT 960
#define WIDTH 1280
// pro mys
double mouseX = 0.0, mouseY = 0.0;
glm::vec2 circleOffset(0.0f, 0.0f);
bool rightPressed = false;
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
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
      glfwCreateWindow(800, 600, "OpenGL Circle", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create window\n";
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

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

  Object sphere1;
  sphere1.VAO = sphereMesh.VAO;
  sphere1.VBO = sphereMesh.VBO;
  sphere1.EBO = sphereMesh.EBO;
  sphere1.indexCount = sphereMesh.indexCount;
  sphere1.modelMatrix =
      glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));

  Object sphere2 = sphere1;
  sphere2.modelMatrix =
      glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, 0.0f, 0.0f));

  plane.objs.push_back(sphere1);
  plane.objs.push_back(sphere2);
  OrthoCamera camera;
  while (!glfwWindowShouldClose(window)) {
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.use();
    shader.setMat4("projection", camera.projection);
    shader.setMat4("view", camera.view);

    plane.draw(shader);

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
  glDeleteVertexArrays(1, &sphereMesh.VAO);
  glDeleteBuffers(1, &sphereMesh.VBO);
  glDeleteBuffers(1, &sphereMesh.EBO);

  glfwTerminate();
  return 0;
}
