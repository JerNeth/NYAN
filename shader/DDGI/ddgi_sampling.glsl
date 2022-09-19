#ifndef DDGI_SAMPLING_GLSL
#define DDGI_SAMPLING_GLSL

#include "ddgi_common.glsl"

//struct DDGIVolume {
//	//Assuming Grid
//	float spacingX;
//	float spacingY;
//	float spacingZ;
//
//	float inverseSpacingX;
//	float inverseSpacingY;
//	float inverseSpacingZ;
//
//	float gridOriginX;
//	float gridOriginY;
//	float gridOriginZ;
//
//	uint probeCountX;
//	uint probeCountY;
//	uint probeCountZ;
//
//	uint raysPerProbe;
//
//	//Probe size in px 
//	uint irradianceProbeSize;
//	uint depthProbeSize;
//
//	uint irradianceTextureSizeX;
//	uint irradianceTextureSizeY;
//	float inverseIrradianceTextureSizeX;
//	float inverseIrradianceTextureSizeY;
//	uint irradianceTextureBinding;
//	uint irradianceTextureSampler;
//	uint irradianceImageBinding;
//
//	uint depthTextureSizeX;
//	uint depthTextureSizeY;
//	float inverseDepthTextureSizeX;
//	float inverseDepthTextureSizeY;
//	uint depthTextureBinding;
//	uint depthTextureSampler;
//	uint depthImageBinding;
//
//	float shadowBias;
//	float maxRayDistance;
//	float hysteresis;
//	float irradianceThreshold;
//	float lightToDarkThreshold;
//};


float get_volume_weight(vec3 worldPos, DDGIVolume volume) {
	return 1.0f;
}

ivec3 get_volume_base_probe(vec3 worldPos, DDGIVolume volume) {
	vec3 position = worldPos - get_volume_origin(volume);
	vec3 spacing = get_volume_spacing(volume);
	ivec3 probeCountsMinusOne = ivec3(volume.probeCountX - 1, volume.probeCountY - 1, volume.probeCountZ - 1);

	ivec3 probeIdx = ivec3(position / spacing);

	probeIdx = clamp(probeIdx, ivec3(0), probeCountsMinusOne);

	return probeIdx;
}
//#define SAMPLE_IRRADIANCE_NEAREST

vec3 sample_ddgi(vec3 worldPos, 
				vec3 bias,
				vec3 direction, 
				DDGIVolume volume) {

	vec3 irradiance = vec3(0.f);
	if(volume.enabled == 0)
		return vec3(0.05f);
	float weightSum = 0.f;

	vec3 biasedWorldPos = worldPos + bias * volume.shadowBias;
	
	vec3 spacing = vec3(volume.spacingX, volume.spacingY, volume.spacingZ);
	ivec3 probeCountsMinusOne = ivec3(volume.probeCountX - 1, volume.probeCountY - 1, volume.probeCountZ - 1);
	ivec3 baseProbeIdx = get_volume_base_probe(biasedWorldPos, volume);
	
	vec3 baseProbeWorldPos = get_probe_coordinates(baseProbeIdx, volume) ;

	vec3 delta = biasedWorldPos - baseProbeWorldPos;
	vec3 alpha = clamp( delta / spacing,vec3(0.f), vec3(1.f));
	#ifdef SAMPLE_IRRADIANCE_NEAREST
	float closestDist = 1e16;
	vec3 closestIrradiance = vec3(0);
	#endif
	for(int probeIndex = 0; probeIndex < 8; probeIndex++)
	{
		ivec3 adjacentProbeIdxOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1, 1, 1);
		ivec3 adjacentProbeIdx = clamp(baseProbeIdx + adjacentProbeIdxOffset, ivec3(0), probeCountsMinusOne);

		vec3 adjacentProbeWorldPos = get_probe_coordinates(adjacentProbeIdx, volume);

		vec3 worldPosToAdjacentProbe = normalize(adjacentProbeWorldPos - worldPos);
		vec3 biasedWorldPosToAdjacentProbe = biasedWorldPos - adjacentProbeWorldPos;
		float biasedWorldPosToAdjacentProbeDist = length(biasedWorldPosToAdjacentProbe);
		biasedWorldPosToAdjacentProbe *= 1.f/ biasedWorldPosToAdjacentProbeDist;

		vec3 trilinear = max(vec3(0.001f), mix(vec3(1.f) - alpha, alpha, vec3(adjacentProbeIdxOffset)));
		float trilinearWeight = trilinear.x *trilinear.y * trilinear.z;
		float weight = 1.f;

		#ifdef WRAPSHADING
		float backFaceWeight = fma(dot(worldPosToAdjacentProbe, direction), 0.5f , 0.5f) ;
		weight *= fma(backFaceWeight, backFaceWeight, 0.2f);
		#else
		float backFaceWeight = fma(dot(worldPosToAdjacentProbe, direction), 0.5f , 0.5f) ;
		weight *= backFaceWeight * backFaceWeight;
		#endif


		vec2 octCoords = get_octahedral_coords(biasedWorldPosToAdjacentProbe);
		
		vec2 depthUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.depthProbeSize, volume);
		octCoords = get_octahedral_coords(direction);
		vec2 irradianceUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.irradianceProbeSize, volume);

		
		vec2 filteredDistance = texture(sampler2D(textures2D[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV).rg;
		vec3 probeIrradiance = texture(sampler2D(textures2D[volume.irradianceTextureBinding], samplers[volume.irradianceTextureSampler]), irradianceUV).rgb;

		float chebyshev = 1.f;
		if(biasedWorldPosToAdjacentProbeDist > filteredDistance.x) {
			float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);
			float v = biasedWorldPosToAdjacentProbeDist - filteredDistance.x;
			chebyshev = variance / (variance + (v * v));
			//Pretty sure we don't need max here since variance is positive and v² is also positive
			chebyshev = chebyshev * chebyshev * chebyshev;
		}
		weight *= max(0.02f, chebyshev);

		weight = max(1e-6f, weight);

//		Originally crush tiny weights because of log perception
		const float threshold = 0.2f;
		if(weight < threshold)
		{
			weight *= weight * weight * (1.f / (threshold * threshold));
		}

		weight *= trilinearWeight;
		//TODO maybe do gamma
		irradiance += weight * probeIrradiance;
		weightSum += weight;

	#ifdef SAMPLE_IRRADIANCE_NEAREST
		if(closestDist > biasedWorldPosToAdjacentProbeDist) {
			closestDist = biasedWorldPosToAdjacentProbeDist;
			closestIrradiance = probeIrradiance;
		}
		#endif
	}
	#ifdef SAMPLE_IRRADIANCE_NEAREST
	return closestIrradiance;
	#endif
	if(weightSum == 0.f) return vec3(0.f);

	irradiance *= 1.f / weightSum; //Normalize
	irradiance *= 3.14159265359 * 2;

	return irradiance;

}

#endif