#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint specularBinding;
    uint specularSampler;
    uint diffuseBinding;
    uint diffuseSampler;
    uint tonemapping;
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51f;
    const float b = 0.03f;
    const float c = 2.43f;
    const float d = 0.59f;
    const float e = 0.14f;
    return clamp(vec3(0.f), vec3(1.f), (x *(a*x+b))/(x*(c*x+d)+e));
}

vec3 UnchartedFilm(vec3 x)
{

	//https://www.slideshare.net/ozlael/hable-john-uncharted2-hdr-lighting
    const float a = 0.22;
    const float b = 0.30f;
    const float c = 0.10f;
    const float d = 0.20f;
    const float e = 0.01f;
    const float f = 0.30f;
    return ((x*(a*x +c*b)+d*e)/(x*(a*x+b)+d*f))- e/f;
}

vec3 reinhard(vec3 x)
{
    return x / (1+x);
}

void main() {
//    vec3 normal = subpassLoad(inputNormal).xyz *2.0 - 1.0;
//    vec3 ambient = 0.1 * constants.lightColor.xyz;
//    float diff = max(dot(normal, constants.lightDir.xyz), 0.0);
//    vec3 diffuse = diff * constants.lightColor.xyz;
//
    //outColor = fromLinear(texture(sampler2D(textures2D[constants.specularBinding], samplers[constants.specularSampler]), inTexCoord) + 
    //           texture(sampler2D(textures2D[constants.diffuseBinding], samplers[constants.diffuseSampler]), inTexCoord));
    vec4 diffuse = texture(sampler2D(textures2D[constants.diffuseBinding], samplers[constants.diffuseSampler]), inTexCoord);
    vec4 specular = texture(sampler2D(textures2D[constants.specularBinding], samplers[constants.specularSampler]), inTexCoord);
    vec4 combined = diffuse + specular;
    if(constants.tonemapping == 1)
        combined.xyz = ACESFilm(combined.xyz);
    else if(constants.tonemapping == 2)
        combined.xyz = reinhard(combined.xyz);
    else if(constants.tonemapping == 3)
        combined.xyz = UnchartedFilm(combined.xyz) / UnchartedFilm(vec3(11.2));
    outColor = fromLinear(combined);
    //outColor = specular;
    //outColor.xyz = outColor.xyz / (1 + outColor.xyz);
    //outColor.xyz = ACESFilm(outColor.xyz);
    
    //outColor = vec4(subpassLoad(inputColor).xyz * (diffuse + ambient), 1.0);
    //outColor = subpassLoad(inputNormal);

    //float depth = pow(subpassLoad(inputDepth).x, 100);
    //outColor = vec4(depth.xxx , 1.0);
    //outColor = vec4(texture(texSampler, inTexCoord).xxx, 1.0);
    // outColor = vec4(gl_FragCoord.xy / vec2(1920, 1080), 0.0, 1.0);
}