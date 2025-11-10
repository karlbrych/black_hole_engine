#pragma once
#include "glm/fwd.hpp"
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include <string.h>
#include <algorithm>
struct Object{
  glm::dvec3 pos;
  double xv,yv,zv;
  long mass;
  float radius;
  bool IsBlackHole;
  GLuint VAO,VBO,EBO;
  size_t indexCount;
  glm::mat4 modelMatrix;
  void draw (const shader& shader)const;
}; 

struct Plane{
    void draw(const shader &shader)const;
    std::vector<Object> objs;
};
double invSqrt(double n);
void DoGravity(Plane &plane);





