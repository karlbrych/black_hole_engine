#pragma once
#include <cstddef>
#include <vector>
#include <glad/glad.h>
struct sphere{
    GLuint VAO,VBO,EBO=0;
    size_t indexCount = 0;
    
}; 
sphere createSphere(float radius,unsigned int sectors,unsigned int stacks);