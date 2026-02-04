#include <cstdio>
#include <emscripten/emscripten.h>
#include <GLFW/glfw3.h>

#include "engine.h"

static GLFWwindow* window = nullptr;
static double lastTime = 0.0;

void loop() {
    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    glViewport(0, 0, fbw, fbh);
    engineSetAspect((float)fbw / (float)fbh);
    double t = glfwGetTime();
    float dt = (lastTime == 0.0) ? 0.0f : float(t - lastTime);
    lastTime = t;

    engineUpdate(dt);
    engineRender();

    glfwSwapBuffers(window);
    glfwPollEvents();
}

int main() {
    printf("WebGL entry\n");

    if (!glfwInit()) return 1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(800, 600, "Black Hole Engine (Web)", nullptr, nullptr);
    if (!window) return 1;

    glfwMakeContextCurrent(window);

    engineInit();

    emscripten_set_main_loop(loop, 0, true);
    return 0;
}
