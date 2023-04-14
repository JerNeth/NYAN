#ifndef DDGI_SAMPLING_GLSL
#define DDGI_SAMPLING_GLSL
#undef RAYTRACE_DDGI_VOLUMES
#ifdef RAYTRACE_DDGI_VOLUMES
#extension GL_EXT_ray_query : require
#endif

#include "ddgi_common.glsl"
#include "moments.glsl"


vec3 get_volume_surface_bias(in vec3 normal,in  vec3 camDir,in  DDGIVolume volume) 
{
	//return (normal * volume.shadowNormalBias) + (camDir * volume.shadowViewBias);
	vec3 spacing = get_volume_spacing(volume);
	float minElement = min(min(spacing.x, spacing.y), spacing.z);
	//Adpated from Scaling Probe-Based Real-Time Global Illumination for Production
	return mix(normal, camDir, 0.8) * (0.75 * minElement * volume.shadowViewBias);
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
				in DDGIVolume volume
				#ifdef RAYTRACE_DDGI_VOLUMES
				,in accelerationStructureEXT acc
				#endif
				) {
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
		
		vec3 worldPosToAdjacentProbe = adjacentProbeWorldPos - worldPos;
		float worldPosToAdjacentProbeDist = length(worldPosToAdjacentProbe);
		worldPosToAdjacentProbe *= 1.f / worldPosToAdjacentProbeDist;
		vec3 biasedWorldPosToAdjacentProbe = biasedWorldPos - adjacentProbeWorldPos;
		float biasedWorldPosToAdjacentProbeDist = length(biasedWorldPosToAdjacentProbe);
		biasedWorldPosToAdjacentProbe *= 1.f/ biasedWorldPosToAdjacentProbeDist;

		vec3 trilinear = max(vec3(0.001f), mix(vec3(1.f) - alpha, alpha, vec3(adjacentProbeIdxOffset)));
		float trilinearWeight = trilinear.x *trilinear.y * trilinear.z;
		float weight = 1.f;
		
		//#define NOWRAPSHADING
		#ifndef NOWRAPSHADING
		//Wrapshading can cause light bleeding, especially with high light intensities
		float backFaceWeight = fma(dot(worldPosToAdjacentProbe, direction), 0.5f , 0.5f) ;
		weight *= fma(backFaceWeight, backFaceWeight, 0.2f);
		#else
		weight *= max(0.001f, dot(worldPosToAdjacentProbe, direction));
		#endif


		vec2 octCoords = get_octahedral_coords(biasedWorldPosToAdjacentProbe);
		
		vec3 depthUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.depthProbeSize, volume);
		octCoords = get_octahedral_coords(direction);
		vec3 irradianceUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.irradianceProbeSize, volume);


		vec3 probeIrradiance = textureLod(sampler2DArray(textures2DArray[volume.irradianceTextureBinding], samplers[volume.irradianceTextureSampler]), irradianceUV, 0).rgb;

		
		#ifdef RAYTRACE_DDGI_VOLUMES
		
		rayQueryEXT rq;
		const float tMin     = 0.001;
		const float tMax = worldPosToAdjacentProbeDist;
		rayQueryInitializeEXT(rq, acc, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xFF, 
					worldPos, tMin, worldPosToAdjacentProbe, tMax);

		// Traverse the acceleration structure and store information about the first intersection (if any)
		rayQueryProceedEXT(rq);

		if (rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionNoneEXT) {
		} else {
			weight = 0;
		}
		#else

		float maxRayDist = get_volume_max_distance(volume); //Normalize distance
		biasedWorldPosToAdjacentProbeDist = biasedWorldPosToAdjacentProbeDist / maxRayDist;
		if(volume.useMoments != 0) {
			vec4 filteredDistance = textureLod(sampler2DArray(textures2DArray[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV, 0).rgba;
			float shadowValue = sample_moments(filteredDistance, biasedWorldPosToAdjacentProbeDist);
			weight *= max(0.05f, shadowValue);
		}
		else {
			vec2 filteredDistance = textureLod(sampler2DArray(textures2DArray[volume.depthTextureBinding], samplers[volume.depthTextureSampler]), depthUV, 0).rg;
			float chebyshev = 1.f;
			if(biasedWorldPosToAdjacentProbeDist > filteredDistance.x) {
				float variance = abs((filteredDistance.x * filteredDistance.x) - filteredDistance.y);
				float v = biasedWorldPosToAdjacentProbeDist - filteredDistance.x;
				chebyshev = variance / (variance + (v * v));
				//Pretty sure we don't need max here since variance is positive and v² is also positive
				chebyshev = chebyshev * chebyshev * chebyshev;
			}
			weight *= max(0.05f, chebyshev);
		}
		#endif

		weight = max(1e-6f, weight);

//		Originally crush tiny weights because of log perception
		const float threshold = 0.2f;
		if(weight < threshold)
		{
			weight *= weight * weight * (1.f / (threshold * threshold));
		}
		#undef DO_GAMMA
		#define DO_GAMMA // I don't like it but it converges way faster than otherwise
		//#define DO_GAMMA2
		#ifdef DO_GAMMA
		probeIrradiance.xyz = pow(probeIrradiance.xyz, vec3(0.5f * 5.f));
		#elif defined(DO_GAMMA2)
		probeIrradiance.xyz = pow(probeIrradiance.xyz, vec3(0.5f));
		#endif
		weight *= trilinearWeight;
		irradiance += weight * probeIrradiance;
		weightSum += weight;

	}
	if(weightSum <= 1e-9) return vec3(0.f);

	irradiance *= 1.f / weightSum; //Normalize
	#ifdef DO_GAMMA
	irradiance *= irradiance; //sRGB blending, I don't like it
	#elif defined(DO_GAMMA2)
	irradiance *= irradiance; //sRGB blending, I don't like it
	#endif	
	irradiance *= 3.14159265359 * 2;

	return irradiance;

}


//#define SAMPLE_DDGI
vec3 diffuse_indirect_lighting(in DDGIVolume volume, in ShadingData shadingData
		#ifdef RAYTRACE_DDGI_VOLUMES
		,in accelerationStructureEXT acc
		#endif
)
{
//#ifdef SAMPLE_DDGI
    float volumeWeight = get_volume_weight(shadingData.worldPos.xyz, volume);
    if(shadingData.metalness < 1.f && volumeWeight > 0.f) {
        vec3 bias = get_volume_surface_bias( shadingData.shadingNormal, shadingData.outLightDir, volume);
        vec3 irradiance = sample_ddgi(shadingData.worldPos.xyz, bias, shadingData.shadingNormal, volume        		
            #ifdef RAYTRACE_DDGI_VOLUMES
            , acc
			#endif
        );
        vec3 radiance = shadingData.albedo.xyz * irradiance * ((1.f - shadingData.metalness) * brdf_lambert() * volumeWeight); //Use Lambert, might be interesting to investigate other BRDFs with split sum, but probably not worth it
        return radiance;
    }
//#endif
    return vec3(0.f);
}
//#define SAMPLE_DDGI
#endif