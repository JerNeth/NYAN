#ifndef DDGI_SAMPLING_GLSL
#define DDGI_SAMPLING_GLSL

#include "ddgi_common.glsl"
#include "moments.glsl"


vec3 get_volume_surface_bias(in vec3 normal,in  vec3 camDir,in  DDGIVolume volume) 
{
	//return (normal * volume.shadowNormalBias) + (camDir * volume.shadowViewBias);
	vec3 spacing = get_volume_spacing(volume);
	float minElement = min(min(spacing.x, spacing.y), spacing.z);
	//Adpated from Scaling Probe-Based Real-Time Global Illumination for Production
	return mix(normal, camDir, 0.8) * (0.75 * minElement * volume.shadowNormalBias);
}

float get_volume_weight(in vec3 worldPos,in  DDGIVolume volume) {
	if(volume.enabled == 0)
		return 0.f;
	vec3 origin = get_volume_origin(volume);
	vec3 extent = (get_volume_spacing(volume) * get_volume_probe_count_minus_one(volume)) * 0.5f;

	vec3 relativePosition = abs(worldPos - (origin+ extent));
	vec3 delta = relativePosition - extent;
	if(all(lessThan(delta, vec3(0)))) return 1.f;
	float weight = 1.f;
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	weight *= 1.f - clamp(delta.x * inverseSpacing.x, 0.f, 1.f);
	weight *= 1.f - clamp(delta.y * inverseSpacing.y, 0.f, 1.f);
	weight *= 1.f - clamp(delta.z * inverseSpacing.z, 0.f, 1.f);
	return weight;
}

ivec3 get_volume_base_probe(in vec3 worldPos,in  DDGIVolume volume) {
	vec3 position = worldPos - get_volume_origin(volume);
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = get_volume_probe_count_minus_one(volume);

	ivec3 probeIdx = ivec3(position * inverseSpacing);

	probeIdx = clamp(probeIdx, ivec3(0), probeCountsMinusOne);

	return probeIdx;
}
//#define SAMPLE_IRRADIANCE_NEAREST

vec3 sample_ddgi(in vec3 worldPos, 
				in vec3 bias,
				in vec3 direction, 
				in DDGIVolume volume) {
	vec3 irradiance = vec3(0.f);
	if(volume.enabled == 0)
		return vec3(0.f);
	float weightSum = 0.f;

	vec3 biasedWorldPos = worldPos + bias;
	
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = get_volume_probe_count_minus_one(volume);
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
		
		#ifndef NOWRAPSHADING
		float backFaceWeight = fma(dot(worldPosToAdjacentProbe, direction), 0.5f , 0.5f) ;
		weight *= fma(backFaceWeight, backFaceWeight, 0.2f);
		#else
		weight *= dot(worldPosToAdjacentProbe, direction);
		#endif


		vec2 octCoords = get_octahedral_coords(biasedWorldPosToAdjacentProbe);
		
		vec2 depthUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.depthProbeSize, volume);
		octCoords = get_octahedral_coords(direction);
		vec2 irradianceUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.irradianceProbeSize, volume);


		vec3 probeIrradiance = textureLod(sampler2D(textures2D[volume.irradianceTextureBinding], samplers[volume.irradianceTextureSampler]), irradianceUV, 0).rgb;

		float maxRayDist = get_volume_max_distance(volume); //Normalize distance
		biasedWorldPosToAdjacentProbeDist = biasedWorldPosToAdjacentProbeDist / maxRayDist;
		if(volume.useMoments != 0) {
			vec4 filteredDistance = textureLod(sampler2D(textures2D[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV, 0).rgba;
			float shadowValue = sample_moments(filteredDistance, biasedWorldPosToAdjacentProbeDist);
			weight *= max(0.05f, shadowValue);
		}
		else {
			vec2 filteredDistance = textureLod(sampler2D(textures2D[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV, 0).rg;
			float chebyshev = 1.f;
			if(biasedWorldPosToAdjacentProbeDist > filteredDistance.x) {
				float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);
				float v = biasedWorldPosToAdjacentProbeDist - filteredDistance.x;
				chebyshev = variance / (variance + (v * v));
				//Pretty sure we don't need max here since variance is positive and v� is also positive
				chebyshev = chebyshev * chebyshev * chebyshev;
			}
			weight *= max(0.05f, chebyshev);
		}

		weight = max(1e-6f, weight);

//		Originally crush tiny weights because of log perception
		const float threshold = 0.2f;
		if(weight < threshold)
		{
			weight *= weight * weight * (1.f / (threshold * threshold));
		}
		#undef DO_GAMMA
		//#define DO_GAMMA
		#ifdef DO_GAMMA
		probeIrradiance.xyz = pow(probeIrradiance.xyz, vec3(0.5f * 5.f));
		#endif		
		probeIrradiance.xyz = pow(probeIrradiance.xyz, vec3(0.5f));
		weight *= trilinearWeight;
		irradiance += weight * probeIrradiance;
		weightSum += weight;

	}
	if(weightSum <= 1e-9) return vec3(0.f);

	irradiance *= 1.f / weightSum; //Normalize
	#ifdef DO_GAMMA
	irradiance *= irradiance; //sRGB blending, I don't like it
	#endif	
	irradiance *= irradiance; //sRGB blending, I don't like it
	irradiance *= 3.14159265359 * 2;

	return irradiance;

}

#endif