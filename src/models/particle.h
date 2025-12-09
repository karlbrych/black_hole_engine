#pragma once
#include <memory>
#include "glm/fwd.hpp"
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include <string.h>
struct Object {
    glm::vec3 pos;
    double xv, yv, zv;
    double mass;
    float radius;
    bool IsBlackHole = false;
    bool dead = false;

    GLuint VAO, VBO, EBO;
    size_t indexCount;

    glm::mat4 modelMatrix;

    void draw(const shader& shader) const;
    void rotate (float time);
};

struct Plane {
    void draw(const shader &shader) const;
    void rotate(float time);
    std::vector<std::unique_ptr<Object>> objs;
};

double invSqrt(double n);
void DoGravity(Plane *plane,double G, double dt);
void MergeAndDeadRemove(Plane *plane, const int *n, std::vector<bool> &dead);




