#include "glm/fwd.hpp"
#include <iostream>
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>

struct Object{
  glm::vec3 pos;
  long speed;
  long mass;
  double radius;
  bool IsBlackHole;
};

struct Plane{
    std::vector<Object> objs;

};
void draw(Object *objects[]);


