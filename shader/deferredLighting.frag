#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint albedoBinding;
    uint albedoSampler;
    uint normalBinding;
    uint normalSampler;
    uint pbrBinding;
    uint pbrSampler;
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

void main() {
//    vec3 normal = subpassLoad(inputNormal).xyz *2.0 - 1.0;
//    vec3 ambient = 0.1 * constants.lightColor.xyz;
//    float diff = max(dot(normal, constants.lightDir.xyz), 0.0);
//    vec3 diffuse = diff * constants.lightColor.xyz;
//
    outSpecular = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inTexCoord);
    vec3 normal = texture(sampler2D(textures2D[constants.normalBinding], samplers[constants.normalSampler]), inTexCoord).xyz;
    outSpecular = vec4(normal, 1);
    outDiffuse = vec4(0, 0, 0, 1);
    
    //outColor = vec4(subpassLoad(inputColor).xyz * (diffuse + ambient), 1.0);
    //outColor = subpassLoad(inputNormal);

    //float depth = pow(subpassLoad(inputDepth).x, 100);
    //outColor = vec4(depth.xxx , 1.0);
    //outColor = vec4(texture(texSampler, inTexCoord).xxx, 1.0);
    // outColor = vec4(gl_FragCoord.xy / vec2(1920, 1080), 0.0, 1.0);
}