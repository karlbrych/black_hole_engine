#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
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
  double mass;  
  size_t indexCount;
  void draw (const shader& shader, glm::mat4 projection, glm::mat4 view)const;
  void rotate (float time);
  float radius;
  GLuint textureId;
  bool IsBlackHole;
  bool IsLightSource = false;
    bool IsPreview = false;
    bool deserialize(std::istream& in) {
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
        in.read(reinterpret_cast<char*>(&IsLightSource), sizeof(IsLightSource));
	modelMatrix = glm::translate(glm::mat4(1.0f), pos);
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
        out.write(reinterpret_cast<const char*>(&IsLightSource), sizeof(IsLightSource));
    }
         Object(double xv_ = 0, double yv_ = 0, double zv_ = 0,
           glm::vec3 pos_ = glm::vec3(0.0f),
           unsigned int VAO_ = 0, unsigned int VBO_ = 0, unsigned int EBO_ = 0,
           double mass_ = 1, int indexCount_ = 0,
           float radius_ = 1.0f, unsigned int textureId_ = 0, bool IsBlackHole_ = false, bool IsLightSource_ = false)
        : xv(xv_), yv(yv_), zv(zv_), pos(pos_), VAO(VAO_), VBO(VBO_), EBO(EBO_),
          mass(mass_), indexCount(indexCount_), radius(radius_), textureId(textureId_), IsBlackHole(IsBlackHole_), IsLightSource(IsLightSource_)
    {
        modelMatrix = glm::translate(glm::mat4(1.0f), pos);
    }
	static Object* deserializeFrom(std::istream& in) {
        Object* obj = new Object();
        if (!obj->deserialize(in)) {
            delete obj;
            return nullptr;
        }
        return obj;
    }
}; 
struct Plane{

    double G = 0.00675;
    double dt = 0.01;
    const size_t version = 1;
    std::vector<glm::vec3> accelBuffer;
    std::vector<int> mergeTargetBuffer;
    void draw(const class shader &shader, const class shader &lightShader, glm::mat4 projection, glm::mat4 view, glm::vec3 cameraPos)const;
	void rotate(float time);
    std::vector<Object*> objs;
};
void load_from_binary(Plane *plane, const std::string &filename);
void save_to_binary(const Plane *plane,const std::string &filename);
double invSqrt(double n);
void DoGravity(Plane *plane,double G, double dt);
void MergeAndDeadRemove(Plane *plane, const int *n, std::vector<bool> &dead);






