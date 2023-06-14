#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_ARB_separate_shader_objects : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "gbuffer.glsl"
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

 const float minLight = 1e-5f;
void point_lights(in Scene scene, in ShadingData shadingData, inout vec3 diffuse, inout vec3 specular) 
{
    for(int i = 0; i < maxNumPointLights; i++) 
    {
        PointLight light = scene.pointLights[i];
        vec3 lightDir = light.pos.xyz - shadingData.worldPos;
        float dist = length(lightDir);
        if(dist > light.attenuationDistance)
            continue;

        lightDir /= dist;
        //float lightShadow = light_visibility(shadingData, lightDir,dist);
        float lightShadow = 1.f;
        if(lightShadow <= minLight )
            continue;
        //Falloff borrowed from Real Shading in Unreal Engine 4, Brian Karis 2013 SIGGRAPH
        float distRatio = dist / light.attenuationDistance;
        distRatio *= distRatio;
        distRatio *= distRatio;
        float falloff = min(max(1.f - distRatio, 0.0f), 1.0f);
        falloff *= falloff;
        falloff *= 1.f / (dist * dist + 1.f);
        LightData lightData;
        lightData.dir = lightDir;
        lightData.intensity = light.intensity * falloff;
        lightData.color = light.color.rgb;
        //calc_light(lightData, shadingData, diffuse, specular);
        diffuse_light(lightData, shadingData, diffuse);
    }
}

void direct_lighting(in Scene scene, in ShadingData shadingData, out vec3 diffuse, out vec3 specular)
{
    diffuse = vec3(0.f);
    specular = vec3(0.f);
    if(scene.dirLight.enabled > 0) 
    {
        float lightShadow = 1.f;
        if(lightShadow > minLight) {
            LightData lightData;
            lightData.dir = -scene.dirLight.dir;
            lightData.intensity = scene.dirLight.intensity;
            lightData.color = scene.dirLight.color;
            //calc_light(lightData, shadingData, diffuse, specular);
            diffuse_light(lightData, shadingData, diffuse);
        }
    }
    if(scene.numPointLights > 0)
    {
        point_lights(scene, shadingData, diffuse, specular);
    }
}

void main() {

//    Scene scene = scenes[constants.sceneBinding].scene;
//	DDGIVolume volume = ddgiVolumes[constants.ddgiBinding].volume[constants.ddgiIndex];
//    
//    uint stencil = texelFetch( usampler2D(utextures2D[constants.stencilBinding], samplers[constants.stencilSampler]), ivec2(gl_FragCoord.xy), 0).x;
//
//    if(stencil == 0) {
//        discard;
//    } else {
//        GBufferData gBufferData;
//        //#define SHOW_NORMALS
//        gBufferData.data0 = texture(sampler2D(textures2D[constants.albedoBinding], samplers[constants.albedoSampler]), inUV);
//        gBufferData.data1 = texture(sampler2D(textures2D[constants.normalBinding], samplers[constants.normalSampler]), inUV);
//        gBufferData.data2 = texture(sampler2D(textures2D[constants.pbrBinding], samplers[constants.pbrSampler]), inUV);
//
//        MaterialData materialData = decode_gbuffer_data(gBufferData);
//        //albedo = vec4(1, 1, 1, 1);
//        float depth = texture(sampler2D(textures2D[constants.depthBinding], samplers[constants.depthBinding]), inTexCoord).x;
//        vec3 normal = get_octahedral_direction(one_to_minus_one(unpack1212(normalTexVal.xyz)));
//
//        vec4 clipSpacePos = vec4(inTexCoord * 2.0 - 1.0, depth, 1.0);
//        vec4 viewSpacePos = scene.invProj * clipSpacePos;
//        viewSpacePos /= viewSpacePos.w;
//        vec4 worldSpacePos = scene.invView * viewSpacePos;
//        
//        vec3 diffuseAccum  = vec3(0.f, 0.f, 0.f);
//        vec3 specularAccum = vec3(0.f, 0.f, 0.f);
//        direct_lighting(scene, shadingData, diffuseAccum.xyz, specularAccum.xyz);
//   
//        
//        //shadeFragment(worldSpacePos.xyz, normal, scene, albedo, metalness, roughness, specular, diffuse);
//        outSpecular = vec4(specular.xyz, 1);
//        outDiffuse = vec4(diffuse.xyz,1);
//        //outSpecular = vec4(normal.xyz,1);
//        //outSpecular = vec4( (pow(1 - depth, 128)).xxx ,1);
//        //outSpecular = albedo;
//
//        //outDiffuse = vec4(depth.xxx / (depth.xxx + 1),1);
//        //outSpecular = vec4(depth.xxx / (depth.xxx + 1),1);
//    }
}