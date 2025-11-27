#include "texture.h"
#include "stb_image.h"
#include <iostream>
#include <unordered_map>

std::unordered_map<std::string, GLuint> Texture::textureCache;
GLuint Texture::LoadTexture(const std::string& path,bool isFliped){
   if (textureCache.find(path)!=textureCache.end()) return textureCache[path];


stbi_set_flip_vertically_on_load(isFliped);


int width, height, channels;
unsigned char* data = stbi_load(path.c_str(), &width, &height, &channels, 0);


if (!data) {
std::cerr << "Failed to load texture: " << path << std::endl;
std::cerr << "Reason: " << stbi_failure_reason() << "\n";
return 0;
}


GLenum format = (channels == 1) ? GL_RED : (channels == 3) ? GL_RGB : GL_RGBA;


GLuint textureID;
glGenTextures(1, &textureID);
glBindTexture(GL_TEXTURE_2D, textureID);
glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
glGenerateMipmap(GL_TEXTURE_2D);


glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


stbi_image_free(data);


textureCache[path] = textureID;
return textureID;
}
void Texture::Bind(GLuint textureID, unsigned int slot) {
if (slot > 31) slot = 31;
glActiveTexture(GL_TEXTURE0 + slot);
glBindTexture(GL_TEXTURE_2D, textureID);
}


// NEW BindTexture method
void Texture::BindTexture(GLuint textureID, unsigned int slot) {
Bind(textureID, slot);
}
bool Texture::isLoaded(const std::string& path){
    return textureCache.find(path) != textureCache.end();
}
