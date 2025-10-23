#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include "models/shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}
GLuint createCircleVAO(float radius, int segments, int *vertexCountOut);
GLuint createCircleVAO(float radius, int segments, int *vertexCountOut);
GLuint VBO,VAO,EBO;
int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Window", nullptr, nullptr);
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
    //uziti nasi krasne shader class
    shader shader("../src/shaders/vertex.glsl","../src/shaders/fragment.glsl");
    glViewport(0, 0, 800, 600);
    int vertexCount;
    GLuint circleVAO = createCircleVAO(0.5, 128, &vertexCount);

    while (!glfwWindowShouldClose(window)) {
        int w,h;
        glfwPollEvents();
        glfwGetFramebufferSize(window,&w,&h);
        glViewport(0,0,w,h);
        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        shader.use();
        glBindVertexArray(circleVAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, vertexCount);

        glfwSwapBuffers(window);
    }
    glDeleteVertexArrays(1,&circleVAO);
    glfwTerminate();
    return 0;
}
//generovani Vertices pro nas vymrdany circle 
float* generateCircleVertices(float radius, int segments, int *vertexCountOut) {
    if (segments < 3) segments = 3;
    int vertexCount = segments + 2; // center + segments + repeat first
    *vertexCountOut = vertexCount;

    float *verts = (float*)malloc(sizeof(float) * vertexCount * 3);

    // center
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
//generovani VAO pro nas circle 
GLuint createCircleVAO(float radius, int segments, int *vertexCountOut) {
    int vertexCount;
    float *vertices = generateCircleVertices(radius, segments, &vertexCount);
    if (!vertices) return 0;

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 3 * sizeof(float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    free(vertices);

    if (vertexCountOut) *vertexCountOut = vertexCount;
    return VAO;
}