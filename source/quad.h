#pragma once 

#include <glad/glad.h>
#include <glm/vec3.hpp>

class quad 
{
private:

    static GLuint m_gl_VAO;
    static GLuint m_gl_VBO;
    static GLuint m_gl_VEO;

    static bool m_bIsInit; 

private:

    static glm::vec3 vertices[];
    static glm::uvec3 indices[];

public:

    static void InitGL();
    static void DeInitGL();
    static void Draw();
};