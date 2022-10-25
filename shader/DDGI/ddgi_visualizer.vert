#version 460
#extension GL_GOOGLE_include_directive : enable

#include "bufferReferences.glsl"
#include "structs.h"
#include "bindlessLayouts.glsl"
#include "extracts.glsl"
#include "ddgi_common.glsl"

layout(std430, push_constant) uniform PushConstants
{
    uint sceneBinding;
	uint ddgiBinding;
	uint ddgiCount;
	uint ddgiIndex;
} constants;


layout(location = 0) out vec3 centerWorldPos;
layout(location = 1) out vec3 worldPos;
layout(location = 2) out uvec3 probeIdx;
layout(location = 3) out vec3 offsetWorldPos;

void main() {
    Scene scene = scenes[constants.sceneBinding].scene;
	DDGIVolume volume = ddgiVolumes[constants.ddgiBinding].volume[constants.ddgiIndex];
	int probeId = gl_VertexIndex / 6;
	int vtxId = gl_VertexIndex % 6;

	

	probeIdx = get_probe_index(probeId, volume);
	centerWorldPos = get_probe_coordinates(probeIdx, volume);
	worldPos = centerWorldPos;
    vec3 viewPos = get_viewer_pos(scene);
	vec3 camUp = vec3(scene.cameraUpX, scene.cameraUpY, scene.cameraUpZ);
	vec3 viewDir = normalize(viewPos - worldPos);
	vec3 right = normalize(cross(viewDir, camUp));
	vec3 up = normalize(cross(viewDir, right));

	if(vtxId == 0 || vtxId == 3) 
	{
		worldPos += (-right + up) * volume.visualizerRadius;
	}
	else if(vtxId == 1)
	{
		worldPos += (right + up)* volume.visualizerRadius;
	}
	else if(vtxId == 2 || vtxId == 4)
	{
		worldPos += (right - up)* volume.visualizerRadius;
	}
	else if(vtxId == 5)
	{
		worldPos += (-right - up)* volume.visualizerRadius;
	}
	offsetWorldPos =worldPos + viewDir * volume.visualizerRadius;
	
	gl_Position = scene.proj * scene.view * vec4(offsetWorldPos, 1.0);


}