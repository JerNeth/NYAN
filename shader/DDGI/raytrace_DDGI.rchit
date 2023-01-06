#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "extracts.glsl"
#include "bindlessLayouts.glsl"
#include "common.glsl"
#include "ddgi_push_constants.h"

layout(location = 0) rayPayloadInEXT PackedPayload pld;

hitAttributeEXT vec2 baryCoord;

void main()
{

    Scene scene = scenes[pushConstants.constants.sceneBinding].scene;
	Mesh mesh = meshData[nonuniformEXT(pushConstants.constants.meshBinding)].meshes[nonuniformEXT(gl_InstanceCustomIndexEXT)];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];
	
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

	#ifdef COMPLEX
	payload.roughness = materialData.roughness;
	payload.metallic = materialData.metalness;
	payload.shadingNormal = materialData.shadingNormal;
	#else
	payload.roughness = 1.f;
	payload.metallic = 0.f;
	payload.shadingNormal = vertexData.normal;
	#endif

	//payload.albedo = vec3(1,0, 0);
	pld = pack_payload(payload);
}