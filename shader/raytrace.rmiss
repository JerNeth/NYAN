#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : enable

#include "raycommon.glsl"
#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "common.glsl"

layout(location = 0) rayPayloadInEXT hitPayload hitValue;

layout(std430, push_constant) uniform PushConstants
{
    uint accBinding;
    uint sceneBinding;
    uint meshBinding;
    uint imageBinding;
	vec4 col;
	vec4 col2;
} constants;


void main()
{
    hitValue.hitValue = constants.col2.xyz;
    //hitValue.hitValue = vec3(0.0, 0.1, 0.3);
}