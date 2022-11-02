#version 450
#extension GL_GOOGLE_include_directive : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "extracts.glsl"
#include "gbuffer.glsl"

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


//void flip_backfacing_normal(inout MaterialData materialData, in VertexData vertexData, in Scene scene) 
//{
//    vec3 viewPos = get_viewer_pos(scene);
//    vec3 viewVec = normalize(viewPos - vertexData.worldPos.xyz);
//
//    if(dot(materialData.shadingNormal, viewVec) < 0) {
//        materialData.shadingNormal = -materialData.shadingNormal;
//    }
//}
//
void main() {
   
    Instance instance = instances[constants.instanceBinding].instances[constants.instanceId];
    uint meshId = instance.meshId & 0x00FFFFFF;
	Mesh mesh = meshData[constants.meshBinding].meshes[meshId];
	Material material = materials[mesh.materialBinding].materials[mesh.materialId];
    Scene scene = scenes[constants.sceneBinding].scene;
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
    GBufferData gbuffer = encode_gbuffer_data(materialData);
        
    outAlbedo = gbuffer.data0;
    outNormal = gbuffer.data1;
    outPBR = gbuffer.data2;
}

