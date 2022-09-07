#version 460
#extension GL_GOOGLE_include_directive : enable

#include "../bufferReferences.glsl"
#include "../structs.h"
#include "../bindlessLayouts.glsl"
#include "../extracts.glsl"
#include "ddgi_common.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint sceneBinding;
	uint ddgiBinding;
	uint ddgiCount;
	uint ddgiIndex;
} constants;

layout(location = 0) in flat vec3 centerWorldPos;
layout(location = 1) in vec3 worldPos;

layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

void main() {
    outSpecular = vec4(0, 0, 0, 1);
    outDiffuse = vec4(0, 1, 0, 1);
}