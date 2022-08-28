#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "../raycommon.glsl"
#include "../bufferReferences.glsl"
#include "../structs.h"
#include "../extracts.glsl"
#include "../bindlessLayouts.glsl"
#include "../common.glsl"
#include "../rayvertex.glsl"

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
	vec4 col2;
} constants;

layout(location = 0) rayPayloadEXT PackedPayload pld;

void main()
{
	Payload payload;
	//TODO maybe use Henyey-Greenstein or Environmentmap
	payload.albedo = constants.col.xyz;
	payload.opacity = constants.col.w;
	payload.hitT = 1e27f;
	payload.hitkind = HitDDGIMiss;

	pld = pack_payload(payload);
}