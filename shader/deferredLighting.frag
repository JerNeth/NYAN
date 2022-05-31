#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "lighting.glsl"

layout(std430, push_constant) uniform PushConstants
{
	uint sceneBinding;
    uint albedoBinding;
    uint albedoSampler;
    uint normalBinding;
    uint normalSampler;
    uint pbrBinding;
    uint pbrSampler;
    uint depthBinding;
    uint depthSampler;
    uint stencilBinding;
    uint stencilSampler;
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

void main() {

    Scene scene = scenes[constants.sceneBinding].scene;
	DirectionalLight light1 = scene.dirLight1;
	DirectionalLight light2 = scene.dirLight2;
    
    uint stencil = texelFetch( usampler2D(utextures2D[constants.stencilBinding], samplers[constants.stencilSampler]), ivec2(gl_FragCoord.xy), 0).x;

    if(stencil == 0) {
        discard;
    } else {
        vec3 normal = texture(sampler2D(textures2D[constants.normalBinding], samplers[constants.normalSampler]), inTexCoord).xyz * 2.0 - 1.0;
        vec4 pbr = texture(sampler2D(textures2D[constants.pbrBinding], samplers[constants.pbrSampler]), inTexCoord);
        vec4 albedo = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inTexCoord);
        float depth = texture(sampler2D(textures2D[constants.depthBinding], samplers[constants.depthBinding]), inTexCoord).x;

  
        float metalness = pbr.x;
        float alpha = pbr.y * pbr.y;
        vec4 clipSpacePos = vec4(inTexCoord * 2.0 - 1.0, depth, 1.0);
        vec4 viewSpacePos = scene.invProj * clipSpacePos;
        viewSpacePos /= viewSpacePos.w;
        vec4 worldSpacePos = scene.invView * viewSpacePos;
        vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
        vec3 viewVec = normalize(viewPos - worldSpacePos.xyz);
        
        vec3 diffuse;
        vec3 specular;

        calcDirLight(albedo.xyz, pbr.xyz, viewVec, normal, light1, specular, diffuse);
        outSpecular = vec4(specular, 1);
        outDiffuse = vec4(diffuse,1);
    }
}