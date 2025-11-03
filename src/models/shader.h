#pragma once 

#include "glm/fwd.hpp"
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
	void setVec2(const std::string &name, float x,float y) const;
	void setVec3(const std::string &name,float x, float y, float z) const; //normalni x,y,z coords
	void setVec2V(const std::string &name, const glm::vec2 &vec) const;  //prijima glm::vec 
	void setVec3V(const std::string &name, const glm::vec3 &vec) const;
	void setMat2(const std::string &name, const glm::mat2 &mat)const;
	void setMat3(const std::string &name, const glm::mat3 &mat)const;
	void setMat4(const std::string &name, const glm::mat4 &mat)const;
	

private:
	void checkCompileErrors(GLuint shader,const std::string& type) const;	
};