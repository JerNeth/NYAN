#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable
#include "../raycommon.glsl"
#include "../bufferReferences.glsl"
#include "../structs.h"
#include "../extracts.glsl"
#include "../bindlessLayouts.glsl"
#include "../common.glsl"

layout(std430, push_constant) uniform PushConstants
{
	uint accBinding;
	uint sceneBinding;
	uint meshBinding;
	uint ddgiBinding;
	uint ddgiCount;
	uint ddgiIndex;
	uint renderTarget;
	vec4 col;
	vec4 randomRotation;
} constants;

layout(location = 0) rayPayloadInEXT PackedPayload pld;
//layout(location = 0) rayPayloadInEXT Payload payload;


hitAttributeEXT vec2 baryCoord;

void main()
{

    Scene scene = scenes[constants.sceneBinding].scene;
	Mesh mesh = meshData[nonuniformEXT(constants.meshBinding)].meshes[nonuniformEXT(gl_InstanceCustomIndexEXT)];
	Material material = materials[nonuniformEXT(mesh.materialBinding)].materials[nonuniformEXT(mesh.materialId)];

    const VertexData vertexData = get_vertex_data(mesh, baryCoord, gl_PrimitiveID, gl_ObjectToWorldEXT);
    const MaterialData materialData = get_material_data(material, vertexData);

	Payload payload;

	payload.albedo = materialData.albedo;
	payload.opacity = materialData.opacity;
	payload.worldPos = vertexData.worldPos;
	payload.metallic = materialData.metalness;
	payload.normal = vertexData.normal;
	payload.roughness = materialData.roughness;
	payload.shadingNormal = materialData.shadingNormal;
	payload.hitT = gl_HitTEXT;
	payload.hitkind = gl_HitKindEXT;
	//payload.albedo = vec3(1,0, 0);
	pld = pack_payload(payload);
}