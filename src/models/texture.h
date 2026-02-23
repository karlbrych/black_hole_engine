#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <unordered_map>
#include <glad/glad.h>


class Texture{
  public:
		  static GLuint LoadTexture(const std::string& path,bool isFliped = true);
		  static void BindTexture(GLuint textureId,unsigned int slot = 0);
		  static bool isLoaded(const std::string& path);
		  static void Bind(GLuint textureId,unsigned int slot);
		  static std::unordered_map<std::string, GLuint> textureCache;
};
#endif




