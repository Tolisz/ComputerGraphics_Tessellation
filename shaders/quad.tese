#version 460 core

// --------------------------------------------
layout(quads, equal_spacing, ccw) in;

out FS_IN
{
    vec3 worldPos;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} o;
// --------------------------------------------

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

uniform int PatchIndex;
uniform int NumOfPatches;  // Number of patches if one row/column

uniform sampler2D TEX_height;
uniform bool DisplacementMapping;
uniform bool DynamicLoD;

float B3( int i, float t)
{
    const vec4 bc = vec4( 1.0f, 3.0f, 3.0f, 1.0f );

    float pow1 = (t == 0.0f && i == 0) ? 1.0f : pow( t, float(i) );
    float pow2 = (t == 1.0f && i == 3) ? 1.0f : pow( 1.0f - t, float(3 - i) );
    return bc[i] * pow1 * pow2;
}

float B2(int i, float t)
{
    const vec3 bc = vec3( 1.0f, 2.0f, 1.0f );

    float pow1 = (t == 0.0f && i == 0) ? 1.0f : pow( t, float(i) );
    float pow2 = (t == 1.0f && i == 2) ? 1.0f : pow( 1.0f - t, float(2 - i) );
    return bc[i] * pow1 * pow2;    
}

vec4 pnt(int i, int j, int offset)
{
    return p[4 * i + j + offset];
}

vec4 BezierPatch(float u, float v)
{
    vec4 pos = vec4( 0.0 );
    int offset = bezierShape * 16;

    for ( int i = 0; i < 4; ++i ) {
        for ( int j = 0; j < 4; ++j ) {
            pos += B3( i, u ) * B3( j, v ) * pnt(i, j, offset);
        }
    }    

    return pos;
}

vec3 BezierPatchNormal(float u, float v, out vec3 T, out vec3 B)
{
    int offset = bezierShape * 16;

    vec3 t1 = vec3(0.0f);
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 4; ++j) {
            t1 += 3 * (pnt(i+1, j, offset).xyz - pnt(i, j, offset).xyz) * B2(i, u) * B3(j, v);
        }
    }

    T = normalize(t1);

    vec3 t2 = vec3(0.0f);
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            t2 += 3 * (pnt(i, j+1, offset).xyz - pnt(i, j, offset).xyz) * B3(i, u) * B2(j, v);
        }
    }

    B = normalize(t2);

    return cross(T, B);
}

float factor(float z) 
{
    const float log_10 = log(10.0f);
    return clamp(-16.0f * (log(-z * 0.01f) / log_10), 1.0f, 100.0f); 
}

void main()
{
    float u = gl_TessCoord.x;
    float omu = 1 - u;
    float v = gl_TessCoord.y;
    float omv = 1 - v;

    // Set texture coordinates
    float OneN = 1.0f / NumOfPatches;
    o.texCoords.x = (PatchIndex / NumOfPatches) / float(NumOfPatches) + u * OneN;
    o.texCoords.y = (PatchIndex % NumOfPatches) / float(NumOfPatches) + v * OneN;

    vec4 pos = BezierPatch(u, v);
    vec3 norm = BezierPatchNormal(u, v, o.tangent, o.bitangent);

    o.worldPos = (model * pos).xyz;
    o.normal = mat3(transpose(inverse(model))) * norm;

    if (DisplacementMapping) {  
        float z = (view * vec4(o.worldPos, 1.0f)).z;
        float LoD = !DynamicLoD ? 0.0f : 6.0f - (log(factor(z)) / log(2.0f));  
        float h = 0.05f * textureLod(TEX_height, o.texCoords, LoD).x;
        o.worldPos += h * o.normal;
    }
    
    gl_Position = projection * view * vec4(o.worldPos, 1.0f);
}