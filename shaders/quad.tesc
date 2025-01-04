#version 460 core

layout(vertices = 4) out;

uniform vec4 outerLevel;
uniform vec2 innerLevel;
uniform bool DynamicLoD;

uniform mat4 model;
layout(std140, binding = 0) uniform Matrices 
{
    mat4 view;
    mat4 projection;
};

#define CONTROL_POINTS_SETS 5
layout(std140, binding = 1) uniform ControlPoints
{
    vec4 p[CONTROL_POINTS_SETS * 16];
};
uniform int bezierShape;

float factor(float z) 
{
    const float log_10 = log(10.0f);
    return clamp(-16.0f * (log(-z * 0.01f) / log_10), 1.0f, 100.0f); 
}

void main()
{
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;

    if (!DynamicLoD) {
        gl_TessLevelOuter[0] = outerLevel.x;
        gl_TessLevelOuter[1] = outerLevel.y;
        gl_TessLevelOuter[2] = outerLevel.z;
        gl_TessLevelOuter[3] = outerLevel.w;
        
        gl_TessLevelInner[0] = innerLevel.x;
        gl_TessLevelInner[1] = innerLevel.y;
    }
    else {
        mat4 VM = view * model;
        vec4 c0 = VM * ((p[0]  + p[3] ) / 2.0f);
        vec4 c1 = VM * ((p[0]  + p[12]) / 2.0f);
        vec4 c2 = VM * ((p[15] + p[12]) / 2.0f);
        vec4 c3 = VM * ((p[3]  + p[15]) / 2.0f);
        vec4 c = (c0 + c1 + c2 + c3) / 4.0f;

        gl_TessLevelOuter[0] = factor(c0.z);
        gl_TessLevelOuter[1] = factor(c1.z);
        gl_TessLevelOuter[2] = factor(c2.z);
        gl_TessLevelOuter[3] = factor(c3.z);

        gl_TessLevelInner[0] = gl_TessLevelInner[1] = factor(c.z);        
    }
}