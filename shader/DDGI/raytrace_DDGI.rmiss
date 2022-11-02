#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "extracts.glsl"
#include "bindlessLayouts.glsl"
#include "common.glsl"

layout(std430, push_constant) uniform PushConstants
{
	uint accBinding;
	uint sceneBinding;
	uint meshBinding;
	uint ddgiBinding;
	uint ddgiCount;
	uint ddgiIndex;
	uint renderTarget;
	vec4 randomRotation;
} constants;

layout(location = 0) rayPayloadInEXT  PackedPayload pld;
//layout(location = 0) rayPayloadInEXT  Payload payload;

void main()
{
    Scene scene = scenes[constants.sceneBinding].scene;
	Payload payload;
	//TODO maybe use Henyey-Greenstein or Environmentmap
	payload.albedo = scene.skyLight.color * scene.skyLight.intensity;
	//payload.albedo = vec3(0.f);
	//When environment 0 like here
	payload.opacity = 1.f;
	payload.hitT = 1e27f;
	payload.hitkind = HitDDGIMiss;

	pld = pack_payload(payload);
}