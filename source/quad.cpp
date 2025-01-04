#include "quad.h"

#include <iostream>

GLuint quad::m_gl_VAO = 0;
GLuint quad::m_gl_VBO = 0;
GLuint quad::m_gl_VEO = 0;

bool quad::m_bIsInit = false; 

glm::vec3 quad::vertices[4] = {
    { 1.0f, 0.0f, -1.0f}, 
    {-1.0f, 0.0f, -1.0f}, 
    {-1.0f, 0.0f,  1.0f}, 
    { 1.0f, 0.0f,  1.0f}, 
};

glm::uvec3 quad::indices[2] = {
    {0, 1, 2}, 
    {2, 0, 3},
};

void quad::InitGL()
{
    if (m_bIsInit) {
        std::cerr << "quad's OpenGL structures are already initialized" << std::endl;
        return; 
    }  

    glGenVertexArrays(1, &m_gl_VAO);
    glGenBuffers(1, &m_gl_VBO);
    glGenBuffers(1, &m_gl_VEO);

    glBindVertexArray(m_gl_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_gl_VBO);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(glm::vec3), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_VEO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 2 * sizeof(glm::uvec3), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);

    m_bIsInit = true;
}

void quad::DeInitGL()
{
    if (!m_bIsInit) {
        std::cerr << "quad's OpenGL structures are not initialized" << std::endl;
        return; 
    }

    glDeleteVertexArrays(1, &m_gl_VAO);
    glDeleteBuffers(1, &m_gl_VBO);
    glDeleteBuffers(1, &m_gl_VEO);

    m_bIsInit = false;
}

void quad::Draw()
{
    glBindVertexArray(m_gl_VAO);
    // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    glDrawArrays(GL_PATCHES, 0, 4);
    glBindVertexArray(0);
}
