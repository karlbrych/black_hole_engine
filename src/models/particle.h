#pragma once
#include "glm/fwd.hpp"
#include <sys/types.h>
#include <vector>
#include <glm/glm.hpp>
#include "shader.h"
#include <string.h>
#include <fstream>
#include <memory>
struct Object{
  glm::mat4 modelMatrix;
  double xv,yv,zv;
  glm::vec3 pos;
  GLuint VAO,VBO,EBO;
  long mass;  
  size_t indexCount;
  void draw (const shader& shader)const;
  void rotate (float time);
  float radius;
  GLuint textureId;
  bool IsBlackHole;
    bool deserialize(std::ifstream& in) {
        // Read modelMatrix
        in.read(reinterpret_cast<char*>(&modelMatrix), sizeof(modelMatrix));
        if (!in) return false;

        // Read velocity (xv, yv, zv)
        in.read(reinterpret_cast<char*>(&xv), sizeof(xv));
        in.read(reinterpret_cast<char*>(&yv), sizeof(yv));
        in.read(reinterpret_cast<char*>(&zv), sizeof(zv));
        if (!in) return false;

        // Read position (pos)
        in.read(reinterpret_cast<char*>(&pos), sizeof(pos));
        if (!in) return false;

        // Read other members
        in.read(reinterpret_cast<char*>(&VAO), sizeof(VAO));
        in.read(reinterpret_cast<char*>(&VBO), sizeof(VBO));
        in.read(reinterpret_cast<char*>(&EBO), sizeof(EBO));
        in.read(reinterpret_cast<char*>(&mass), sizeof(mass));
        in.read(reinterpret_cast<char*>(&indexCount), sizeof(indexCount));
        in.read(reinterpret_cast<char*>(&radius), sizeof(radius));
        in.read(reinterpret_cast<char*>(&textureId), sizeof(textureId));
        in.read(reinterpret_cast<char*>(&IsBlackHole), sizeof(IsBlackHole));
        
        return in.good(); // If everything is good, return true
    }
      void serialize(std::ofstream &out) const {
        out.write(reinterpret_cast<const char*>(&modelMatrix), sizeof(modelMatrix));
        out.write(reinterpret_cast<const char*>(&xv), sizeof(xv));
        out.write(reinterpret_cast<const char*>(&yv), sizeof(yv));
        out.write(reinterpret_cast<const char*>(&zv), sizeof(zv));
        out.write(reinterpret_cast<const char*>(&pos), sizeof(pos));
        out.write(reinterpret_cast<const char*>(&VAO), sizeof(VAO));
        out.write(reinterpret_cast<const char*>(&VBO), sizeof(VBO));
        out.write(reinterpret_cast<const char*>(&EBO), sizeof(EBO));
        out.write(reinterpret_cast<const char*>(&mass), sizeof(mass));
        out.write(reinterpret_cast<const char*>(&indexCount), sizeof(indexCount));
        out.write(reinterpret_cast<const char*>(&radius), sizeof(radius));
        out.write(reinterpret_cast<const char*>(&textureId), sizeof(textureId));
        out.write(reinterpret_cast<const char*>(&IsBlackHole), sizeof(IsBlackHole));
    }
}; 
struct Plane{
    const size_t version = 8;
    void draw(const shader &shader)const;
	void rotate(float time);
    std::vector<Object*> objs;
};
void load_from_binary(Plane *plane, const std::string &filename);
void save_to_binary(const Plane *plane,const std::string &filename);
double invSqrt(double n);
void DoGravity(Plane *plane,double G, double dt);
void MergeAndDeadRemove(Plane *plane, const int *n, std::vector<bool> &dead);






