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
layout(location = 1) in flat vec3 fragTangent;
layout(location = 2) in flat vec3 fragNormal;
layout(location = 3) in flat vec3 fragBitangent;

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outNormal;
layout(location = 2) out vec4 outPBR;

void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    vec4 albedo = texture(sampler2D(textures2D[material.albedoTexId], samplers[material.albedoSampler]), fragTexCoord);
    vec3 normal = vec3(texture(sampler2D(textures2D[material.normalTexId], samplers[material.normalSampler]), fragTexCoord).rg, 0) ;
    normal.z = sqrt(1-normal.x*normal.x + normal.y*normal.y);
	mat3 tangentFrame = mat3(fragTangent, fragBitangent, fragNormal);

    normal = normalize(tangentFrame * normal);
    normal = pack1212(encodeOctahedronMapping(normal));
    //normal.xy = encodeOctahedronMapping(normal);

    outAlbedo = albedo;
    outNormal = vec4(normal.xyz, material.roughness);
    //outNormal = normal.xy;
    outPBR = vec4(material.metalness, material.F0_R, material.F0_G, material.F0_B);
    outPBR.yzw -= vec3(0.022, 0.022, 0.022);
    //outColor = vec4(0.2,0.6,0.5,1.0);
}

