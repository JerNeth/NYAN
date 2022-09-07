#ifndef DDGI_COMMON_GLSL
#define DDGI_COMMON_GLSL
#include "../structs.h"

uvec3 get_probe_index(uint probeIdx, DDGIVolume volume) 
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

uint get_probe_index(uvec3 probeIdx, DDGIVolume volume) 
{
	const uint zStride = volume.probeCountX * volume.probeCountY;
	const uint yStride = volume.probeCountX;
	return probeIdx.z * zStride + probeIdx.y*yStride +probeIdx.x;
}

vec3 get_volume_origin(DDGIVolume volume) 
{
	return vec3(volume.gridOriginX, volume.gridOriginY, volume.gridOriginZ);
}

vec3 get_volume_spacing(DDGIVolume volume) 
{
	return vec3(volume.spacingX, volume.spacingY, volume.spacingZ);
}

vec3 get_probe_coordinates(uvec3 idx, DDGIVolume volume) 
{
	vec3 origin = get_volume_origin(volume);
	vec3 spacing = get_volume_spacing(volume);
	return origin + idx * spacing;
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

vec3 get_ray_direction(uint rayIdx, DDGIVolume volume)
{
	return spherical_fibonacci(rayIdx, volume.raysPerProbe);
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

ivec2 get_probe_texel_coords(ivec3 probeIdx, in int probeTexelSize, in DDGIVolume volume) {
	probeIdx.y += int(volume.probeCountY) * probeIdx.z;
	return 1 + probeIdx.xy *  ivec2(2+ probeTexelSize);
}


vec2 sign_not_zero(in vec2 v)
{
	return fma(step(0, v),vec2(2.f),vec2(-1.f));
}

//[-1, 1] domain
vec3 get_octahedral_direction(in vec2 uv) 
{
	vec3 direction = vec3(uv.xy, 1.f - abs(uv.x) - abs(uv.y));
	if(direction.z < 0.f)
	{
		direction.xy = (vec2(1.f) - abs(uv.yx)) * sign_not_zero(direction.xy);
	}
	return normalize(direction);
}
//[-1, 1] domain
vec2 get_octahedral_coords(in vec3 direction)
{
    vec2 uv= direction.xy *  (1.f / dot(abs(direction), vec3(1.f)));
    uv = mix(uv, (1.0 - abs(uv.yx)) * sign_not_zero(uv), step(direction.z, 0.0));
	return uv;

}

vec2 get_probe_uv(in ivec3 probeIdx, in vec2 octahedralCoords, in uint probeTexelSize, in DDGIVolume volume)
{
	ivec2 texelCoords = get_probe_texel_coords(probeIdx, int(probeTexelSize), volume);

	vec2 uv = texelCoords + (octahedralCoords * 0.5 + 0.5) * probeTexelSize;
	vec2 texSize = vec2(volume.probeCountX, volume.probeCountY * volume.probeCountZ) * (probeTexelSize + 2);
	return uv / texSize;
}
#endif