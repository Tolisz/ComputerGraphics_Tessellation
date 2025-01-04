#version 460 core 

#define CONTROL_POINTS_SETS 5

layout(std140, binding = 0) uniform Matrices 
{
    mat4 view;
    mat4 projection;
};

uniform mat4 model;

layout(std140, binding = 1) uniform ControlPoints
{
    // 0 - flat, 1 - up, 2 - down
    vec4 p[CONTROL_POINTS_SETS * 16];
};

uniform int bezierShape;

void main() 
{
    gl_Position = projection * view * model * p[gl_VertexID + 16 * bezierShape];
}