#pragma once
#include "glm/fwd.hpp"
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include <string.h>
#include <memory>
struct Object{
  glm::vec3 pos;
  double xv,yv,zv;
  long mass;
  float radius;
  bool IsBlackHole;
  GLuint VAO,VBO,EBO;
  size_t indexCount;
  glm::mat4 modelMatrix;
  GLuint textureId;
  void draw (const shader& shader)const;
  void rotate (float time);
}; 

struct Plane{
    void draw(const shader &shader)const;
	void rotate(float time);
    std::vector<Object*> objs;
};
double invSqrt(double n);
void DoGravity(Plane *plane,double G, double dt);
void MergeAndDeadRemove(Plane *plane, const int *n, std::vector<bool> &dead);





