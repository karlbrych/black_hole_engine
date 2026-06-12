#include "texture.h"
#include "stb_image.h"
#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>

std::unordered_map<std::string, GLuint> Texture::textureCache;

GLuint Texture::LoadPreprocessedTexture(const std::string &path)
{
  std::ifstream in(path, std::ios::binary);
  if (!in.is_open())
  {
    std::cerr << "Failed to open preprocessed texture: " << path << std::endl;
    return 0;
  }

  auto readU32 = [&in](uint32_t &v) -> bool {
    in.read(reinterpret_cast<char *>(&v), sizeof(v));
    return static_cast<bool>(in);
  };

  uint32_t magic = 0, version = 0, width = 0, height = 0, channels = 0, mipLevels = 0;
  if (!readU32(magic) || magic != 0x42585458)
  {
    std::cerr << "Invalid preprocessed texture magic in: " << path << std::endl;
    return 0;
  }
  if (!readU32(version) || !readU32(width) || !readU32(height) || !readU32(channels) || !readU32(mipLevels))
  {
    std::cerr << "Failed to read preprocessed texture header: " << path << std::endl;
    return 0;
  }

  GLenum format = (channels == 1) ? GL_RED : (channels == 3) ? GL_RGB : GL_RGBA;

  GLuint textureID = 0;
  glGenTextures(1, &textureID);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  for (uint32_t level = 0; level < mipLevels; ++level)
  {
    uint32_t lw = 0, lh = 0, dataSize = 0;
    if (!readU32(lw) || !readU32(lh) || !readU32(dataSize))
    {
      std::cerr << "Failed to read mip level header from: " << path << std::endl;
      glDeleteTextures(1, &textureID);
      return 0;
    }

    std::vector<unsigned char> data(dataSize);
    in.read(reinterpret_cast<char *>(data.data()), static_cast<std::streamsize>(dataSize));
    if (!in)
    {
      std::cerr << "Failed to read mip level data from: " << path << std::endl;
      glDeleteTextures(1, &textureID);
      return 0;
    }

    glTexImage2D(GL_TEXTURE_2D,
                 static_cast<GLint>(level),
                 format,
                 static_cast<GLsizei>(lw),
                 static_cast<GLsizei>(lh),
                 0,
                 format,
                 GL_UNSIGNED_BYTE,
                 data.data());
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  mipLevels > 1 ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return textureID;
}

GLuint Texture::LoadTexture(const std::string &path, bool isFliped)
{
  auto it = textureCache.find(path);
  if (it != textureCache.end())
    return it->second;

  // Prefer preprocessed .bhtx file if it exists
  std::filesystem::path prePath(path);
  prePath.replace_extension(".bhtx");
  if (std::filesystem::exists(prePath))
  {
    GLuint preTex = LoadPreprocessedTexture(prePath.string());
    if (preTex != 0)
    {
      std::cout<<"Loaded preprocessed texture: " << prePath.string() << std::endl;
     
      textureCache[path] = preTex;
      return preTex;
    }
  }
  std::cout<<"loading normal texture: " << path << " as " << prePath.string() << std::endl;
  stbi_set_flip_vertically_on_load(isFliped);

  int width, height, channels;
  unsigned char *data = stbi_load(path.c_str(), &width, &height, &channels, 0);

  if (!data)
  {
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

void Texture::CleanupAll() {
    for (auto& [path, id] : textureCache) {
        if (id != 0) {
            glDeleteTextures(1, &id);
        }
    }
    textureCache.clear();
}
