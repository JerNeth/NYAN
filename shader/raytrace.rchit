#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable
#extension GL_GOOGLE_include_directive : enable
#include "raycommon.glsl"

layout(location = 0) rayPayloadInEXT hitPayload hitValue;
hitAttributeEXT vec3 attribs;

void main()
{
	hitValue.hitValue = vec3(0.2, 0.5, 0.5);
}
