#ifndef SHADERSTRUCTS_GLSL
#define SHADERSTRUCTS_GLSL
struct VertexData
{
    vec3 tangent;
    vec3 normal;
    vec3 bitangent;
    vec3 worldPos;
    vec2 uv;
};


struct MaterialData
{
    float metalness;
    float roughness;
    vec3 albedo;
    float opacity;
    vec3 shadingNormal;
    vec3 emissive;

};

struct ShadingData
{
    vec3 albedo;
    float alpha;
    vec3 outLightDir;
    float metalness;
    vec3 shadingNormal;
    //float anisotropy (maybe)
    vec3 worldPos;
};
struct LightData
{
    vec3 dir;
    float intensity;
    vec3 color;
};

#endif
