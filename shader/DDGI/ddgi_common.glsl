#ifndef DDGI_COMMON_GLSL
#define DDGI_COMMON_GLSL
#include "bindlessLayouts.glsl"
#include "structs.h"

uvec3 get_probe_index(in uint probeIdx,in DDGIVolume volume) 
{
	const uint zStride = volume.probeCountX * volume.probeCountY;
	const uint yStride = volume.probeCountX;
	const uint z = probeIdx / zStride;
	probeIdx -= z*zStride;
	const uint y = probeIdx / yStride;
	probeIdx -= y*yStride;
	const uint x = probeIdx;
	return uvec3(x, y, z);
}

uint get_probe_index(in uvec3 probeIdx, in DDGIVolume volume) 
{
	const uint zStride = volume.probeCountX * volume.probeCountY;
	const uint yStride = volume.probeCountX;
	return probeIdx.z * zStride + probeIdx.y*yStride +probeIdx.x;
}

vec3 get_volume_origin(DDGIVolume volume) 
{
	return vec3(volume.gridOriginX, volume.gridOriginY, volume.gridOriginZ);
}

vec3 get_volume_inverse_spacing(DDGIVolume volume) 
{
	return vec3(volume.inverseSpacingX, volume.inverseSpacingY, volume.inverseSpacingZ);
}

vec3 get_volume_spacing(DDGIVolume volume) 
{
	return vec3(volume.spacingX, volume.spacingY, volume.spacingZ);
}

uvec3 get_volume_probe_count(DDGIVolume volume) 
{
	return uvec3(volume.probeCountX, volume.probeCountY, volume.probeCountZ);
}

ivec3 get_volume_probe_count_minus_one(DDGIVolume volume) 
{
	return ivec3(volume.probeCountX- 1, volume.probeCountY- 1, volume.probeCountZ- 1);
}

vec3 get_probe_offset(in uint probeIdx, in DDGIVolume volume) 
{
	return vec3(ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3],
		ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 1],
		ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 2]);
}


vec3 get_probe_offset(in uvec3 idx, in DDGIVolume volume) 
{
	uint probeIdx = get_probe_index(idx, volume);
	return get_probe_offset(probeIdx, volume);
}

void set_probe_offset(in uint probeIdx, in DDGIVolume volume, in vec3 offset) 
{
	ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3] = offset.x;
	ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 1] = offset.y;
	ddgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 2] = offset.z;
}

vec3 get_probe_coordinates(in uvec3 idx,in DDGIVolume volume)
{
	vec3 origin = get_volume_origin(volume);
	vec3 spacing = get_volume_spacing(volume);
	vec3 offset = get_probe_offset(idx, volume);
	return origin + offset + idx * spacing;
}


vec3 get_probe_coordinates(uint probeIdx, DDGIVolume volume) 
{
	uvec3 idx = get_probe_index(probeIdx, volume);
	return get_probe_coordinates(idx, volume);
}


vec3 spherical_fibonacci(float i, float n) {
	const float pi = 3.14159265359f;
    const float PHI = sqrt(5) * 0.5 + 0.5;
#define madfrac(A, B) ((A)*(B)-floor((A)*(B)))
    float phi = 2.0 * pi * madfrac(i, PHI - 1);
    float cosTheta = 1.0 - (2.0 * i + 1.0) * (1.0 / n);
    float sinTheta = sqrt(max(min(1.0 - cosTheta*cosTheta, 1.0), 0.0));

    return vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta);

#undef madfrac
}

vec3 get_ray_direction(in vec4 randomRotation, in uint rayIdx, in DDGIVolume volume)
{
	float maxCount = volume.raysPerProbe;
	float idx = rayIdx;
	bool fixedRay = false;
	if(volume.relocationEnabled != 0 || volume.classificationEnabled != 0) {
		fixedRay = rayIdx < volume.fixedRayCount;
		idx = mix( idx - volume.fixedRayCount, idx, fixedRay);
		maxCount = mix(maxCount - volume.fixedRayCount, volume.fixedRayCount, fixedRay);
	}
	vec3 dir = spherical_fibonacci(idx, maxCount);
    if(fixedRay)
		return dir;

	return quaternion_rotate(randomRotation, dir);
}

float get_volume_max_distance(in DDGIVolume volume)
{
	return length(get_volume_spacing(volume)) * 1.5f; //Get maximum possible distance
}

vec2 get_normalized_octahedral_coords(ivec2 texCoords, int numTexels) 
{
	vec2 uv = vec2(texCoords.x % numTexels, texCoords.y % numTexels);

	uv += vec2(0.5f);
	uv /= vec2(numTexels);
	uv *= 2.f;
	uv -= vec2(1.f);
	return uv;
}

ivec2 get_probe_texel_coords(ivec3 probeIdx, in ivec2 probeTexelSize, in DDGIVolume volume) {
	probeIdx.y += int(volume.probeCountY) * probeIdx.z;
	return 1 + probeIdx.xy *  (2+ probeTexelSize);
}

vec2 get_probe_uv(in ivec3 probeIdx, in vec2 octahedralCoords, in uint probeTexelSize, in DDGIVolume volume)
{
	ivec2 texelCoords = get_probe_texel_coords(probeIdx, ivec2(probeTexelSize), volume);

	vec2 uv = texelCoords + (octahedralCoords * 0.5 + 0.5) * probeTexelSize;
	vec2 texSize = vec2(volume.probeCountX, volume.probeCountY * volume.probeCountZ) * (probeTexelSize + 2);
	return uv / texSize;
}
#endif