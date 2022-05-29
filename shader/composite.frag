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
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;

void main() {
//    vec3 normal = subpassLoad(inputNormal).xyz *2.0 - 1.0;
//    vec3 ambient = 0.1 * constants.lightColor.xyz;
//    float diff = max(dot(normal, constants.lightDir.xyz), 0.0);
//    vec3 diffuse = diff * constants.lightColor.xyz;
//
    outColor = fromLinear(texture(sampler2D(textures2D[constants.specularBinding], samplers[constants.specularSampler]), inTexCoord) + 
               texture(sampler2D(textures2D[constants.diffuseBinding], samplers[constants.diffuseSampler]), inTexCoord)) ;
    
    
    //outColor = vec4(subpassLoad(inputColor).xyz * (diffuse + ambient), 1.0);
    //outColor = subpassLoad(inputNormal);

    //float depth = pow(subpassLoad(inputDepth).x, 100);
    //outColor = vec4(depth.xxx , 1.0);
    //outColor = vec4(texture(texSampler, inTexCoord).xxx, 1.0);
    // outColor = vec4(gl_FragCoord.xy / vec2(1920, 1080), 0.0, 1.0);
}