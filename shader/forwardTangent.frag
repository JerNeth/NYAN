#version 460
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_ray_query : require

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "extracts.glsl"
#include "lighting.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint instanceBinding;
    uint instanceId;
    uint sceneBinding;
    uint accBinding;
} constants;


layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[ACC_COUNT];
layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
//layout(location = 2) in vec3 fragTangent;
//layout(location = 3) in vec3 fragBitangent;
layout(location = 4) in vec3 fragWorldPos;

layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;


void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 albedo = texture(sampler2D(textures2D[material.albedoTexId], samplers[material.albedoSampler]), fragTexCoord);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    vec2 normalSample = texture(sampler2D(textures2D[material.normalTexId], samplers[material.normalSampler]), fragTexCoord).rg;
   // vec3 normal = tangentSpaceNormal(normalSample,fragNormal, fragBitangent, fragTangent);
    Scene scene = scenes[constants.sceneBinding].scene;
    
//    vec3 position = gl_FragCoord.xyz / gl_FragCoord.w;
//    vec3 tmpNormal = fragNormal;
//    vec3 tmpBitangent;
//    vec3 tmpTangent;
//    calculateTBN(tmpTangent, tmpBitangent, tmpNormal, position, fragTexCoord);
//    vec3 normal = tangentSpaceNormal(normalSample, tmpNormal, tmpBitangent, tmpTangent);
//    normal = pack1212(encodeOctahedronMapping(normalize(normal)));
    vec3 normal = fragNormal;
    float roughness = material.roughness;
    float metalness = material.metalness;
    vec4 diffuse;
    vec4 specular;
    
    //rayQueryEXT rq;

    //rayQueryInitializeEXT(rq, accelerationStructures[constants.accBinding], gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, fragWorldPos.xyz, 0.01, scene.dirLight1.dir, 10000);

    // Traverse the acceleration structure and store information about the first intersection (if any)
   // rayQueryProceedEXT(rq);

    //if (rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionNoneEXT) {
        // Not in shadow
        shadeFragment(fragWorldPos, normal, scene, albedo, metalness, roughness, specular, diffuse);
        outSpecular = vec4(specular.xyz, albedo.w);
        outDiffuse = vec4( diffuse.xyz, albedo.w);
//    }
//    else {
//        outSpecular = vec4(0, 0,0 , albedo.w);
//        outDiffuse = vec4(0, 0,0 , albedo.w);
//    }
    //normal.xy = encodeOctahedronMapping(normal);
//
//    outAlbedo = albedo;
//    outNormal = vec4(normal.xyz, material.roughness);
//    //outNormal = normal.xy;
//    outPBR = vec4(material.metalness, material.F0_R, material.F0_G, material.F0_B);
//    outPBR.yzw -= vec3(0.022, 0.022, 0.022);
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

