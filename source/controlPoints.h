#pragma once

#include <glad/glad.h>
#include <glm/vec2.hpp>

class controlPoints
{
private:

    static glm::uvec2 indices[];
    
    GLuint m_gl_VAO;
    GLuint m_gl_VEO;

public:

    void InitGL();
    void Draw();
    void DeInitGL();

};