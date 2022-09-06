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
    uint ddgiBinding;
    uint ddgiCount;
    uint ddgiIndex;
} constants;

layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

void main() {

    Scene scene = scenes[constants.sceneBinding].scene;
	DDGIVolume volume = ddgiVolumes[constants.ddgiBinding].volume[constants.ddgiIndex];
    
    uint stencil = texelFetch( usampler2D(utextures2D[constants.stencilBinding], samplers[constants.stencilSampler]), ivec2(gl_FragCoord.xy), 0).x;

    if(stencil == 0) {
        discard;
    } else {
        vec4 normalTexVal = texture(sampler2D(textures2D[constants.normalBinding], samplers[constants.normalSampler]), inTexCoord);
        vec4 pbr = texture(sampler2D(textures2D[constants.pbrBinding], samplers[constants.pbrSampler]), inTexCoord);
        vec4 albedo = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inTexCoord);
        //albedo = vec4(1, 1, 1, 1);
        float depth = texture(sampler2D(textures2D[constants.depthBinding], samplers[constants.depthBinding]), inTexCoord).x;
        vec3 normal = decodeOctahedronMapping(unpack1212(normalTexVal.xyz));

  
        float metalness = pbr.x;
        vec3 F0 = pbr.yzw + vec3(0.022, 0.022, 0.022); //Add 0.022 for e.g. gold
        float roughness = normalTexVal.w;
        vec4 clipSpacePos = vec4(inTexCoord * 2.0 - 1.0, depth, 1.0);
        vec4 viewSpacePos = scene.invProj * clipSpacePos;
        viewSpacePos /= viewSpacePos.w;
        vec4 worldSpacePos = scene.invView * viewSpacePos;
        vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
        vec3 viewVec = normalize(viewPos - worldSpacePos.xyz);
//        if(dot(normal, viewVec) <= 0) {
//            normal = -normal;
//        }
        
        vec4 diffuse = vec4(0.0);
        vec4 specular= vec4(0.0);
        
        shadeFragment(worldSpacePos.xyz, normal, scene, albedo, metalness, roughness, specular, diffuse);
        outSpecular = vec4(specular.xyz, 1);
        outDiffuse = vec4(diffuse.xyz,1);
        //outSpecular = vec4(normal.xyz,1);
        //outSpecular = vec4( (pow(1 - depth, 128)).xxx ,1);
        //outSpecular = albedo;

        //outDiffuse = vec4(depth.xxx / (depth.xxx + 1),1);
        //outSpecular = vec4(depth.xxx / (depth.xxx + 1),1);
    }
}