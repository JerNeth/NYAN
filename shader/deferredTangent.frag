#version 450
#extension GL_GOOGLE_include_directive : enable
#extension GL_EXT_nonuniform_qualifier : enable

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


void flip_backfacing_normal(inout VertexData vertexData) 
{
    if(!gl_FrontFacing) {
        vertexData.normal = -vertexData.normal;
        vertexData.tangent = -vertexData.tangent;
        vertexData.bitangent = -vertexData.bitangent;
    }
}

void main() {
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    Scene scene = scenes[constants.sceneBinding].scene;
    VertexData vertexData;
    vertexData.normal = normalize(fragNormal);
    vertexData.tangent = fragTangent.xyz;
    orthonormalize(vertexData.normal, vertexData.tangent, vertexData.bitangent);
    //vertexData.bitangent *= fragTangent.w;
    vertexData.worldPos = fragWorldPos;
    vertexData.uv = fragTexCoord;
    
    computeTangentSpace(vertexData, dFdxFine(fragWorldPos), dFdyFine(fragWorldPos), dFdxFine(vertexData.uv),  dFdyFine(vertexData.uv));

    if((material.flags & MATERIAL_DOUBLE_SIDED_FLAG) == MATERIAL_DOUBLE_SIDED_FLAG)
        flip_backfacing_normal(vertexData);


    MaterialData materialData = get_material_data(material, vertexData);


    outAlbedo = vec4(materialData.albedo, materialData.opacity);
    outNormal = vec4(pack1212(zero_to_one(get_octahedral_coords(materialData.shadingNormal))), materialData.roughness);
    //outNormal = normal.xy;
    outPBR = vec4(materialData.metalness, materialData.emissive.xyz);
    //outColor = vec4(0.2,0.6,0.5,1.0);
}


    
//    vec3 dPdx = dFdxFine(fragWorldPos);
//    vec3 dPdy = dFdyFine(fragWorldPos);
//    vec3 sigmaX = dPdx - dot(dPdx, vertexData.normal) * vertexData.normal;
//    vec3 sigmaY = dPdy - dot(dPdy, vertexData.normal) * vertexData.normal;
//    float flipSign = sign_not_zero(dot(dPdy, cross(vertexData.normal, dPdx)));
//
//    vec2 dSTdx = dFdxFine(vertexData.uv);
//    vec2 dSTdy = dFdyFine(vertexData.uv);
//
//    float det = dot(dSTdx, vec2(dSTdy.y, - dSTdy.x));
//    float signDet = sign_not_zero(det);
//
//    vec2 invC0 = signDet * vec2(dSTdy.y, -dSTdx.y);
//    vec3 vT = sigmaX * invC0.x + sigmaY * invC0.y;
//    if(abs(det) > 0.0) vT = normalize(vT);
//    vec3 vB = (signDet * flipSign) * cross(vertexData.normal, vT);

//
//    vertexData.tangent = vT;
//    vertexData.bitangent = vB;