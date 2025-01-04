#version 460 core 

// --------------------------------------------
layout(location = 0) out vec4 FragColor;

in FS_IN
{
    vec3 worldPos;
    vec2 texCoords;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} i;
// --------------------------------------------

struct Material
{
    vec3 ka;
    vec3 kd;
    vec3 ks;
    float shininess;
};

struct Light
{
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    vec3 position;
};

uniform Material mat;
uniform Light lgt;

uniform vec3 viewPos; 

uniform bool UsePhong;
uniform bool Texturing;
uniform bool NormalMapping;

uniform sampler2D TEX_diffuse;
uniform sampler2D TEX_normals; 

vec3 PhongIllumination(vec3 worldPos, vec3 N, vec3 V, vec3 surfaceColor)
{
    vec3 color = mat.ka * lgt.ambient;

    // diffuse 
    vec3 L = normalize(lgt.position - worldPos);
    color += mat.kd * surfaceColor * max(dot(N, L), 0.0f);

    // specular
    vec3 R = reflect(-L, N);
    color += mat.ks * lgt.specular * pow(max(dot(R, V), 0.0f), mat.shininess);

    return color;
}  

void main() 
{
    vec4 surfaceColor = Texturing ? texture(TEX_diffuse, i.texCoords) : vec4(lgt.diffuse, 1.0f);

    if (!UsePhong) {
        FragColor = surfaceColor;
        return;
    }

    vec3 N = normalize(i.normal);
    vec3 T = normalize(i.tangent);
    vec3 B = normalize(i.bitangent);

    if (NormalMapping) {
        vec4 tN = 2.0f * texture(TEX_normals, i.texCoords) - 1.0f;
        tN.y *= -1.0f;
        N = tN.x * T + tN.y * B + tN.z * N; 
    }

    vec3 V = normalize(viewPos - i.worldPos);

    vec3 color = PhongIllumination(i.worldPos, N, V, surfaceColor.xyz);
    FragColor = vec4(color, 1.0f);
}