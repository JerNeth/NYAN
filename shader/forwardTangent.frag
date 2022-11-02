#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_query : require
#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "extracts.glsl"
#include "lighting.glsl"
#include "DDGI/ddgi_sampling.glsl"
#include "gbuffer.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint instanceBinding;
    uint instanceId;
    uint sceneBinding;
    uint accBinding;
    uint ddgiBinding;
    uint ddgiCount;
    uint ddgiIndex;
} constants;


layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[ACC_COUNT];
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragTangent;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;


float light_visibility(in ShadingData shadingData, in vec3 dir, in float tMax) 
{
    rayQueryEXT rq;
    const float tMin     = 0.01;

    rayQueryInitializeEXT(rq, accelerationStructures[constants.accBinding], gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xFF, 
                shadingData.worldPos.xyz + shadingData.shadingNormal * 0.02, tMin, dir.xyz, tMax);

    // Traverse the acceleration structure and store information about the first intersection (if any)
    rayQueryProceedEXT(rq);

    if (rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionNoneEXT) {
        return 1.0f;
    } else {
        return 0.f;
    }
}
const float minLight = 1e-5;
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
        float lightShadow = light_visibility(shadingData, lightDir,dist);
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
        calc_light(lightData, shadingData, diffuse, specular);
    }
}

void direct_lighting(in Scene scene, in ShadingData shadingData, out vec3 diffuse, out vec3 specular)
{
    diffuse = vec3(0.f);
    specular = vec3(0.f);
    if(scene.dirLight.enabled > 0) 
    {
        float lightShadow = light_visibility(shadingData, -scene.dirLight.dir, 1e27f);
        if(lightShadow > minLight) {
            LightData lightData;
            lightData.dir = -scene.dirLight.dir;
            lightData.intensity = scene.dirLight.intensity;
            lightData.color = scene.dirLight.color;
            calc_light(lightData, shadingData, diffuse, specular);
        }
    }
    if(scene.numPointLights > 0)
    {
        point_lights(scene, shadingData, diffuse, specular);
    }
}

void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    Scene scene = scenes[constants.sceneBinding].scene;
	DDGIVolume volume = ddgiVolumes[constants.ddgiBinding].volume[constants.ddgiIndex];
    VertexData vertexData;
    vertexData.normal = fragNormal;
    vertexData.tangent = fragTangent.xyz;
    orthonormalize(vertexData.normal, vertexData.tangent, vertexData.bitangent);
    vertexData.bitangent *= fragTangent.w;
    vertexData.worldPos = fragWorldPos;
    vertexData.uv = fragTexCoord;
    
    //computeTangentSpace(vertexData, dFdxFine(fragWorldPos), dFdyFine(fragWorldPos), dFdxFine(vertexData.uv),  dFdyFine(vertexData.uv));
    
    flip_backfacing_normal(vertexData, ((material.flags & MATERIAL_DOUBLE_SIDED_FLAG) == MATERIAL_DOUBLE_SIDED_FLAG)
                                        && !gl_FrontFacing);


    MaterialData materialData = get_material_data(material, vertexData);
    
    if(materialData.opacity <= material.alphaDiscard)
        discard;
    vec4 diffuseAccum  = vec4(0.0);
    vec4 specularAccum = vec4(0.0);
    ShadingData shadingData;
    shadingData.albedo = materialData.albedo.xyz;
    shadingData.alpha = materialData.roughness * materialData.roughness;
    shadingData.worldPos = fragWorldPos.xyz;
    shadingData.outLightDir = normalize(get_viewer_pos(scene) - shadingData.worldPos.xyz);
    shadingData.metalness = materialData.metalness;
    shadingData.shadingNormal = materialData.shadingNormal;
    direct_lighting(scene, shadingData, diffuseAccum.xyz, specularAccum.xyz);
        
    float volumeWeight = get_volume_weight(shadingData.worldPos.xyz, volume);
    if(shadingData.metalness < 1.f && volumeWeight > 0.f) {
           
        vec3 bias = get_volume_surface_bias( shadingData.shadingNormal, shadingData.outLightDir, volume);
        vec3 irradiance = sample_ddgi(shadingData.worldPos.xyz,bias, shadingData.shadingNormal, volume);
        vec3 radiance = shadingData.albedo.xyz * irradiance *brdf_lambert() * volumeWeight; //Use Lambert, might be interesting to investigate other BRDFs with split sum, but probably not worth it
        diffuseAccum.xyz += radiance;
    }
    specularAccum.xyz += materialData.emissive.xyz;

    outDiffuse = vec4(diffuseAccum.xyz, materialData.opacity);
    outSpecular = vec4(specularAccum.xyz, materialData.opacity);
}

