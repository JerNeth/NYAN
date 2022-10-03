#ifndef DDGI_SAMPLING_GLSL
#define DDGI_SAMPLING_GLSL

#include "ddgi_common.glsl"
#include "../moments.glsl"

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
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = ivec3(volume.probeCountX - 1, volume.probeCountY - 1, volume.probeCountZ - 1);

	ivec3 probeIdx = ivec3(position * inverseSpacing);

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
	
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = ivec3(volume.probeCountX - 1, volume.probeCountY - 1, volume.probeCountZ - 1);
	ivec3 baseProbeIdx = get_volume_base_probe(biasedWorldPos, volume);
	
	vec3 baseProbeWorldPos = get_probe_coordinates(baseProbeIdx, volume) ;

	vec3 delta = biasedWorldPos - baseProbeWorldPos;
	vec3 alpha = clamp( delta * inverseSpacing,vec3(0.f), vec3(1.f));


	for(int probeIndex = 0; probeIndex < 8; probeIndex++)
	{
		ivec3 adjacentProbeIdxOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1);
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


		vec3 probeIrradiance = texture(sampler2D(textures2D[volume.irradianceTextureBinding], samplers[volume.irradianceTextureSampler]), irradianceUV).rgb;

		float maxRayDist = get_volume_max_distance(volume); //Normalize distance
		biasedWorldPosToAdjacentProbeDist = biasedWorldPosToAdjacentProbeDist / maxRayDist;
		if(volume.useMoments != 0) {
			vec4 filteredDistance = texture(sampler2D(textures2D[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV).rgba;
			float shadowValue = sample_moments(filteredDistance, biasedWorldPosToAdjacentProbeDist);
			weight *= max(0.02f, shadowValue);
		}
		else {
			vec2 filteredDistance = texture(sampler2D(textures2D[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV).rg;
			float chebyshev = 1.f;
			if(biasedWorldPosToAdjacentProbeDist > filteredDistance.x) {
				float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);
				float v = biasedWorldPosToAdjacentProbeDist - filteredDistance.x;
				chebyshev = variance / (variance + (v * v));
				//Pretty sure we don't need max here since variance is positive and v� is also positive
				chebyshev = chebyshev * chebyshev * chebyshev;
			}
			weight *= max(0.02f, chebyshev);
		}

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

	}
	if(weightSum == 0.f) return vec3(0.f);

	irradiance *= 1.f / weightSum; //Normalize
	irradiance *= 3.14159265359 * 2;

	return irradiance;

}

#endif