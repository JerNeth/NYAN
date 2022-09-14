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
	vec4 randomRotation;
} constants;

layout(location = 0) in flat vec3 centerWorldPos;
layout(location = 1) in vec3 worldPos;
layout(location = 2) in flat uvec3 probeIdx;
layout(location = 3) in vec3 offsetWorldPos;

layout(location = 0) out vec4 outSpecular;
layout(location = 1) out vec4 outDiffuse;

// https://www.iquilezles.org/www/articles/spherefunctions/spherefunctions.htm
float sphIntersect( vec3 ro, vec3 rd, vec4 sph )
{
    vec3 oc = ro - sph.xyz;
    float b = dot( oc, rd );
    float c = dot( oc, oc ) - sph.w*sph.w;
    float h = b*b - c;
    if( h<0.0 ) return -1.0;
    h = sqrt( h );
    return -b - h;
}

void main() {
	DDGIVolume volume = ddgiVolumes[constants.ddgiBinding].volume[constants.ddgiIndex];
    Scene scene = scenes[constants.sceneBinding].scene;
    vec3 viewPos = vec3(scene.viewerPosX, scene.viewerPosY, scene.viewerPosZ);
    vec3 dir = normalize(offsetWorldPos - viewPos);
	float radius = volume.visualizerRadius;
    float rayHit = sphIntersect(viewPos, dir, vec4(centerWorldPos, radius));
    if( rayHit == -1)
        discard;
    vec3 surfacePos = dir * rayHit + viewPos;
    vec3 sphereDir = normalize(surfacePos - centerWorldPos);
    
	vec2 octCoords = get_octahedral_coords(sphereDir);
	vec2 irradianceUV = get_probe_uv(ivec3(probeIdx), octCoords, volume.irradianceProbeSize, volume);

	vec3 probeIrradiance = texture(sampler2D(textures2D[volume.irradianceTextureBinding], samplers[volume.irradianceTextureSampler]), irradianceUV).rgb;

    for(int i = 0; i < volume.raysPerProbe; i++) {
        vec3 direction = quaternion_rotate(constants.randomRotation, get_ray_direction(i, volume));
        if(dot(sphereDir, direction) > 0.99999)  {
            probeIrradiance = vec3(1.0f);
            break;
        }
    }
    
    vec4 pos = vec4(scene.proj * scene.view * vec4(surfacePos, 1.0));
	gl_FragDepth = pos.z / pos.w;

    outSpecular =  vec4(0, 0, 0, 0);
    outDiffuse =vec4(probeIrradiance, 1); 
   // outDiffuse = vec4(0, 1, 0, alpha);
}