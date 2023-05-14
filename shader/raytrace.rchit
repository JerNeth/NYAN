#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "extracts.glsl"
#include "common.glsl"
#include "lighting.glsl"
#include "raycommon.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint accBinding;
    uint sceneBinding;
    uint meshBinding;
    uint diffuseImageBinding;
    uint specularImageBinding;
    uint rngSeed;
    uint frameCount;
} constants;

layout(location = 0) rayPayloadInEXT PackedPayload pld;


hitAttributeEXT vec2 baryCoord;

void main()
{

    Scene scene = scenes[constants.sceneBinding].scene;
	Mesh mesh = meshData[constants.meshBinding].meshes[nonuniformEXT(gl_InstanceCustomIndexEXT)];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];
	
	#define COMPLEX
	#ifdef COMPLEX
    VertexData vertexData = get_vertex_data(mesh, baryCoord, gl_PrimitiveID, gl_ObjectToWorldEXT);
	#else
    VertexData vertexData = get_vertex_data_simple(mesh, baryCoord, gl_PrimitiveID, gl_ObjectToWorldEXT);
	#endif
    flip_backfacing_normal(vertexData, ((material.flags & MATERIAL_DOUBLE_SIDED_FLAG) == MATERIAL_DOUBLE_SIDED_FLAG) &&
										(gl_HitKindEXT == gl_HitKindBackFacingTriangleEXT));
	#ifdef COMPLEX
    const MaterialData materialData = get_material_data(material, vertexData);
	#else
	const MaterialData materialData = get_albedo_data(material, vertexData);
	#endif
	Payload payload;

	payload.albedo = materialData.albedo;
	payload.opacity = materialData.opacity;
	payload.worldPos = vertexData.worldPos;
	payload.normal = vertexData.normal;
	payload.hitT = gl_HitTEXT;
	payload.hitkind = gl_HitKindEXT;
	payload.emissive = materialData.emissive;
	#ifdef COMPLEX
	payload.roughness = materialData.roughness;
	payload.metallic = materialData.metalness;
	payload.shadingNormal = materialData.shadingNormal;
	#else
	payload.roughness = 1.f;
	payload.metallic = 0.f;
	payload.shadingNormal = vertexData.normal;
	#endif
	if(payload.hitkind == gl_HitKindBackFacingTriangleEXT &&
	((material.flags & MATERIAL_DOUBLE_SIDED_FLAG) == MATERIAL_DOUBLE_SIDED_FLAG))
		payload.hitkind = gl_HitKindFrontFacingTriangleEXT;

	pld = pack_payload(payload);
}