#ifndef DDGI_RESTIR_SAMPLING_GLSL
#define DDGI_RESTIR_SAMPLING_GLSL
#ifdef RAYTRACE_DDGI_RESTIR_VOLUMES
#extension GL_EXT_ray_query : require
#endif

#include "ddgi_restir_common.glsl"

float get_volume_weight(in vec3 worldPos,in  DDGIReSTIRVolume volume) {
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

ivec3 get_volume_base_probe(in vec3 worldPos,in DDGIReSTIRVolume volume) {
	vec3 position = worldPos - get_volume_origin(volume);
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = get_volume_probe_count_minus_one(volume);

	ivec3 probeIdx = ivec3(position * inverseSpacing);

	probeIdx = clamp(probeIdx, ivec3(0), probeCountsMinusOne);

	return probeIdx;
}

vec3 sample_ddgi_restir(in vec3 worldPos,
				in vec3 direction, 
				in DDGIReSTIRVolume volume
				#ifdef RAYTRACE_DDGI_RESTIR_VOLUMES
				,in accelerationStructureEXT acc
				#endif
				) {
	vec3 irradiance = vec3(0.f);
	if(volume.enabled == 0)
		return vec3(0.f);
	float weightSum = 0.f;
	
	vec3 inverseSpacing = get_volume_inverse_spacing(volume);
	ivec3 probeCountsMinusOne = get_volume_probe_count_minus_one(volume);
	ivec3 baseProbeIdx = get_volume_base_probe(worldPos, volume);
	
	vec3 baseProbeWorldPos = get_probe_coordinates(baseProbeIdx, volume) ;

	vec3 delta = worldPos - baseProbeWorldPos;
	vec3 alpha = clamp( delta * inverseSpacing,vec3(0.f), vec3(1.f));


	for(int probeIndex = 0; probeIndex < 8; probeIndex++)
	{
		ivec3 adjacentProbeIdxOffset = ivec3(probeIndex, probeIndex >> 1, probeIndex >> 2) & ivec3(1);
		ivec3 adjacentProbeIdx = clamp(baseProbeIdx + adjacentProbeIdxOffset, ivec3(0), probeCountsMinusOne);

		vec3 adjacentProbeWorldPos = get_probe_coordinates(adjacentProbeIdx, volume);
		
		vec3 worldPosToAdjacentProbe = adjacentProbeWorldPos - worldPos;
		float worldPosToAdjacentProbeDist = length(worldPosToAdjacentProbe);
		worldPosToAdjacentProbe *= 1.f / worldPosToAdjacentProbeDist;

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


		vec2 octCoords = get_octahedral_coords(worldPosToAdjacentProbe);
		octCoords = get_octahedral_coords(direction);
		const vec3 texelDir = get_octahedral_direction(octCoords);

		vec3 irradianceUV = get_probe_uv(adjacentProbeIdx, octCoords, volume.irradianceProbeSize, volume);


		vec3 probeIrradiance = textureLod(sampler2DArray(textures2DArray[volume.irradianceTextureBinding], samplers[volume.irradianceSamplerBinding]), irradianceUV, 0).rgb;

		
		#ifdef RAYTRACE_DDGI_RESTIR_VOLUMES
		
		rayQueryEXT rq;
		const float tMin     = 0.1;
		const float tMax = worldPosToAdjacentProbeDist;
		rayQueryInitializeEXT(rq, acc, gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT , 0xFF, 
					worldPos, tMin, worldPosToAdjacentProbe, tMax);
					//worldPos, tMin, worldPosToAdjacentProbe + 0.1 * direction, tMax);

		// Traverse the acceleration structure and store information about the first intersection (if any)
		rayQueryProceedEXT(rq);

		if (rayQueryGetIntersectionTypeEXT(rq, true) == gl_RayQueryCommittedIntersectionNoneEXT) {
		} else {
			weight = 0;
			continue;
		}
		#else
		#endif
		const float threshold = 0.2f;
		if(weight < threshold)
		{
			weight *= weight * weight * (1.f / (threshold * threshold));
		}
		weight = max(1e-6f, weight);

		weight *= trilinearWeight;
		irradiance += weight * probeIrradiance;
		weightSum += weight;

	}
	if(weightSum <= 1e-9) return vec3(0.f);

	irradiance *= 1.f / weightSum; //Normalize
//	#ifdef DO_GAMMA
//	irradiance *= irradiance; //sRGB blending, I don't like it
//	#endif	
//	irradiance *= irradiance; //sRGB blending, I don't like it
//	irradiance *= 3.14159265359 * 2;

	return irradiance;

}


#define SAMPLE_DDGI
vec3 diffuse_indirect_lighting(in DDGIReSTIRVolume volume, in ShadingData shadingData
		#ifdef RAYTRACE_DDGI_RESTIR_VOLUMES
		,in accelerationStructureEXT acc
		#endif
)
{
#ifdef SAMPLE_DDGI
    float volumeWeight = get_volume_weight(shadingData.worldPos.xyz, volume);
    if(shadingData.metalness < 1.f && volumeWeight > 0.f) {
        vec3 irradiance = sample_ddgi_restir(shadingData.worldPos.xyz, shadingData.shadingNormal, volume        		
            #ifdef RAYTRACE_DDGI_RESTIR_VOLUMES
            , acc
			#endif
        );
        vec3 radiance = shadingData.albedo.xyz * irradiance * ((1.f - shadingData.metalness) * brdf_lambert() * volumeWeight); //Use Lambert, might be interesting to investigate other BRDFs with split sum, but probably not worth it
        return radiance;
    }
#endif
    return vec3(0.f);
}
//#define SAMPLE_DDGI

#endif