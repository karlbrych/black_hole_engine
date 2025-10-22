#pragma once 
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
class shader
{
public:
	GLuint ID;
	shader(const char *vertexPath,const char *fragmentPath);
	void use() const;
	void setBool(const std::string& name, bool value) const;
	void setFloat(const std::string& name, float value)const;
	void setInt(const std::string& name, int value)const;
private:
	void checkCompileErrors(GLuint shader,const std::string& type) const;	
};