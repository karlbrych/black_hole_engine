#include <stdio.h>
#include "glm/ext/matrix_transform.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>
int WIDTH = 1280;
int HEIGHT = 960;
struct OrthoCamera {
  glm::mat4 projection = glm::ortho(-WIDTH / 20.0f,  // left
                                    WIDTH / 20.0f,   // right
                                    -HEIGHT / 20.0f, // bottom
                                    HEIGHT / 20.0f,  // top
                                    -500.0f,          // near
                                    500.0f            // far
  );
  glm::mat4 view =
      glm::lookAt(glm::vec3(0.0f, 200.0f, 0.001f), // top-down camera
                  glm::vec3(0.0f, 0.0f, 0.0f),   // looking at origin
                  glm::vec3(0.0f, 0.0f, 1.0f)   // “up” points along -Z
      );
};

// Perspective camera suitable for 3D rendering
struct PerspectiveCamera {
  glm::mat4 projection = glm::perspective(
      glm::radians(45.0f),               // field of view in degrees
      (float)WIDTH / (float)HEIGHT,      // aspect ratio
      0.1f,                               // near clipping plane
      1000.0f                             // far clipping plane
  );
  glm::mat4 view =
      glm::lookAt(glm::vec3(0.0f, 0.0f, 5.0f), // camera position
                  glm::vec3(0.0f, 0.0f, 0.0f), // looking at origin
                  glm::vec3(0.0f, 1.0f, 0.0f)  // “up” points along +Y
      );
};
