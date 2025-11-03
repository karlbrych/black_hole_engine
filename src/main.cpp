#include "models/shader.h"
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <iostream>

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

float *generateCircleVertices(float radius, int segments, int *vertexCountOut) {
  if (segments < 3)
    segments = 3;
  int vertexCount = segments + 2;
  *vertexCountOut = vertexCount;

  float *verts = (float *)malloc(sizeof(float) * vertexCount * 3);

  verts[0] = 0.0f;
  verts[1] = 0.0f;
  verts[2] = 0.0f;

  for (int i = 0; i <= segments; ++i) {
    float angle = (2.0f * M_PI * i) / segments;
    int idx = (i + 1) * 3;
    verts[idx + 0] = cosf(angle) * radius;
    verts[idx + 1] = sinf(angle) * radius;
    verts[idx + 2] = 0.0f;
  }

  return verts;
}

GLuint createCircleVAO(float radius, int segments, int *vertexCountOut) {
  int vertexCount;
  float *vertices = generateCircleVertices(radius, segments, &vertexCount);
  if (!vertices)
    return 0;

  GLuint VAO, VBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices,
               GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);

  free(vertices);

  if (vertexCountOut)
    *vertexCountOut = vertexCount;
  return VAO;
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

  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_position_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);

  // Shader
  shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");

  int vertexCount;
  GLuint circleVAO = createCircleVAO(0.25f, 128, &vertexCount);

  // Render loop
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    int w, h;
    glfwGetFramebufferSize(window, &w, &h);
    glViewport(0, 0, w, h);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Update circle position only if right mouse is pressed
    if (rightPressed) {
      int fbWidth, fbHeight;
      glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

      float xNDC = (float)mouseX / fbWidth * 2.0f - 1.0f;
      float yNDC = 1.0f - (float)mouseY / fbHeight * 2.0f;
      circleOffset = glm::vec2(xNDC, yNDC);
    }

    shader.use();
    shader.setVec2V("uOffset", circleOffset);
    glBindVertexArray(circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

    glfwSwapBuffers(window);
  }

  glDeleteVertexArrays(1, &circleVAO);
  glfwTerminate();
  return 0;
}
