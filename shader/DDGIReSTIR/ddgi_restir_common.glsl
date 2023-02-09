#ifndef DDGIRESTIR_COMMON_GLSL
#define DDGIRESTIR_COMMON_GLSL
#include "bindlessLayouts.glsl"
#include "structs.h"

uvec3 get_probe_index(in uint probeIdx,in DDGIReSTIRVolume volume) 
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

uint get_probe_index(in uvec3 probeIdx, in DDGIReSTIRVolume volume) 
{
	const uint zStride = volume.probeCountX * volume.probeCountY;
	const uint yStride = volume.probeCountX;
	return probeIdx.z * zStride + probeIdx.y*yStride +probeIdx.x;
}

vec3 get_volume_origin(DDGIReSTIRVolume volume) 
{
	return vec3(volume.gridOriginX, volume.gridOriginY, volume.gridOriginZ);
}

vec3 get_volume_inverse_spacing(DDGIReSTIRVolume volume) 
{
	return vec3(volume.inverseSpacingX, volume.inverseSpacingY, volume.inverseSpacingZ);
}

vec3 get_volume_spacing(DDGIReSTIRVolume volume) 
{
	return vec3(volume.spacingX, volume.spacingY, volume.spacingZ);
}

uvec3 get_volume_probe_count(DDGIReSTIRVolume volume) 
{
	return uvec3(volume.probeCountX, volume.probeCountY, volume.probeCountZ);
}

ivec3 get_volume_probe_count_minus_one(DDGIReSTIRVolume volume) 
{
	return ivec3(volume.probeCountX- 1, volume.probeCountY- 1, volume.probeCountZ- 1);
}

//vec3 get_probe_offset(in uint probeIdx, in DDGIReSTIRVolume volume) 
//{
//	return vec3(readDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3],
//		readDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 1],
//		readDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 2]);
//}
//
//
//vec3 get_probe_offset(in uvec3 idx, in DDGIReSTIRVolume volume) 
//{
//	uint probeIdx = get_probe_index(idx, volume);
//	return get_probe_offset(probeIdx, volume);
//}
//
//void set_probe_offset(in uint probeIdx, in DDGIReSTIRVolume volume, in vec3 offset) 
//{
//	writeDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3] = offset.x;
//	writeDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 1] = offset.y;
//	writeDdgiOffsets[volume.offsetBufferBinding].offsets[probeIdx * 3 + 2] = offset.z;
//}

vec3 get_probe_coordinates(in uvec3 idx,in DDGIReSTIRVolume volume)
{
	vec3 origin = get_volume_origin(volume);
	vec3 spacing = get_volume_spacing(volume);
	//vec3 offset = get_probe_offset(idx, volume);
	//return origin + offset + idx * spacing;
	return origin + idx * spacing;
}


vec3 get_probe_coordinates(uint probeIdx, DDGIReSTIRVolume volume) 
{
	uvec3 idx = get_probe_index(probeIdx, volume);
	return get_probe_coordinates(idx, volume);
}

vec3 get_ray_direction(in vec4 randomRotation, in uint rayIdx, in uint rayCount)
{
	float maxCount = rayCount;
	float idx = rayIdx;
//	bool fixedRay = false;
//	if(volume.relocationEnabled != 0 || volume.classificationEnabled != 0) {
//		fixedRay = rayIdx < volume.fixedRayCount;
//		idx = mix( idx - volume.fixedRayCount, idx, fixedRay);
//		maxCount = mix(maxCount - volume.fixedRayCount, volume.fixedRayCount, fixedRay);
//	}
	vec3 dir = spherical_fibonacci(idx, maxCount);
    //if(fixedRay)
	//	return dir;

	return quaternion_rotate(randomRotation, dir);
}

float get_volume_max_distance(in DDGIReSTIRVolume volume)
{
	return length(get_volume_spacing(volume)) * 1.5f; //Get maximum possible distance
}

ivec3 get_probe_texel_coords(ivec3 probeIdx, in ivec2 probeTexelSize, in DDGIReSTIRVolume volume) {
	return ivec3(1 + probeIdx.xy *  (2+ probeTexelSize), probeIdx.z);
}

vec3 get_probe_uv(in ivec3 probeIdx, in vec2 octahedralCoords, in uint probeTexelSize, in DDGIReSTIRVolume volume)
{
	ivec3 texelCoords = get_probe_texel_coords(probeIdx, ivec2(probeTexelSize), volume);

	vec2 uv = texelCoords.xy + (octahedralCoords * 0.5 + 0.5) * probeTexelSize;
	vec2 texSize = vec2(volume.probeCountX, volume.probeCountY) * (probeTexelSize + 2);
	return vec3(uv / texSize, texelCoords.z);
}

#endif