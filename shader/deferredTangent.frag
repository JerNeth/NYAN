#version 450
#extension GL_GOOGLE_include_directive : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "extracts.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint meshBinding;
    uint instanceBinding;
    uint instanceId;
    uint sceneBinding;
} constants;


layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec4 fragTangent;
layout(location = 4) in vec3 fragWorldPos; //currently unused

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPBR;


void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 albedo = texture(sampler2D(textures2D[material.albedoTexId], samplers[material.albedoSampler]), fragTexCoord);
    albedo *= fromSRGB(vec4(material.albedo_R, material.albedo_G, material.albedo_B, material.albedo_A));
    vec2 normalSample = texture(sampler2D(textures2D[material.normalTexId], samplers[material.normalSampler]), fragTexCoord).rg;
    vec3 normal = computeTangentSpaceNormal(normalSample,fragNormal, fragTangent);

//    vec3 position = gl_FragCoord.xyz / gl_FragCoord.w;
//    vec3 tmpNormal = fragNormal;
//    vec3 tmpBitangent;
//    vec3 tmpTangent;
//    calculateTBN(tmpTangent, tmpBitangent, tmpNormal, position, fragTexCoord);
//    vec3 normal = tangentSpaceNormal(normalSample, tmpNormal, tmpBitangent, tmpTangent);
    normal = pack1212(encodeOctahedronMapping(normalize(normal)));
    //vec3 normal = pack1212(encodeOctahedronMapping(normalize(fragNormal)));
    //normal.xy = encodeOctahedronMapping(normal);

    outAlbedo = albedo;
    outNormal = vec4(normal.xyz, material.roughness);
    //outNormal = normal.xy;
    outPBR = vec4(material.metalness, 0, 0, 0);
    outPBR.yzw -= vec3(0.022, 0.022, 0.022);
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

