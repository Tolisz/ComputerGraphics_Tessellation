#include "controlPoints.h"

glm::uvec2 controlPoints::indices[] = {
    { 0,  1}, { 1,  2}, { 2,  3},
    { 4,  5}, { 5,  6}, { 6,  7},
    { 8,  9}, { 9, 10}, {10, 11},
    {12, 13}, {13, 14}, {14, 15},

    { 0,  4}, { 1,  5}, { 2,  6}, { 3,  7}, 
    { 8,  4}, { 9,  5}, {10,  6}, {11,  7}, 
    { 8, 12}, { 9, 13}, {10, 14}, {11, 15}, 
};

void controlPoints::InitGL()
{
    glGenVertexArrays(1, &m_gl_VAO);
    glGenBuffers(1, &m_gl_VEO);

    glBindVertexArray(m_gl_VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_gl_VEO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void controlPoints::Draw()
{
    glBindVertexArray(m_gl_VAO);
    glDrawElements(GL_LINES, 48, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void controlPoints::DeInitGL()
{

}
