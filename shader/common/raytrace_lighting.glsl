#ifndef RAYTRACE_LIGHTING_GLSL
#define RAYTRACE_LIGHTING_GLSL

#ifndef SHADOW_RAY_PAYLOAD_LOCATION
#define SHADOW_RAY_PAYLOAD_TEMPORARY_DEFINE
#define SHADOW_RAY_PAYLOAD_LOCATION 1

#endif

layout(location = SHADOW_RAY_PAYLOAD_LOCATION) rayPayloadEXT float visibility;

float light_visibility(in accelerationStructureEXT accelerationStructure, in ShadingData shadingData, in vec3 dir, in float tMax) 
{
    visibility = 0.f;
    uint  rayFlags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    float tMin     = 0.02;
	traceRayEXT(accelerationStructure, // acceleration structure
            rayFlags,       // rayFlagsaccelerationStructure
            0xFF,           // cullMask
            0,              // sbtRecordOffset
            0,              // sbtRecordStride
            1,              // missIndex
            shadingData.worldPos.xyz + shadingData.shadingNormal * 0.02, // ray origin
            tMin,           // ray min range
            dir.xyz,  // ray direction
            tMax,           // ray max range
            SHADOW_RAY_PAYLOAD_LOCATION               // payload (location = 0)
        );
    return visibility;
    //return 1.0f;
}

const float minLight = 1e-5;

vec3 diffuse_point_lights(in accelerationStructureEXT accelerationStructure, in Scene scene, in ShadingData shadingData) 
{
    vec3 diffuse = vec3(0.f);
    for(int i = 0; i < maxNumPointLights; i++) 
    {
        PointLight light = scene.pointLights[i];
        vec3 lightDir = light.pos.xyz - shadingData.worldPos;
        float dist = length(lightDir);
        if(dist > light.attenuationDistance)
            continue;

        lightDir /= dist;
        float lightShadow = light_visibility(accelerationStructure, shadingData, lightDir,dist);
        if(lightShadow <= minLight )
            continue;
        //Falloff borrowed from Real Shading in Unreal Engine 4, Brian Karis 2013 SIGGRAPH
        float distRatio = dist / light.attenuationDistance;
        distRatio *= distRatio;
        distRatio *= distRatio;
        float falloff = clamp(1.f - distRatio, 0.f, 1.f);
        falloff *= falloff;
        falloff *= 1.f / (dist * dist + 1.f);
        LightData lightData;
        lightData.dir = lightDir;
        lightData.intensity = light.intensity * falloff;
        lightData.color = light.color.rgb;
        diffuse_light(lightData, shadingData, diffuse);
        
    }
    return diffuse;
}

vec3 diffuse_direct_lighting(in accelerationStructureEXT accelerationStructure, in Scene scene, in ShadingData shadingData)
{
    vec3 diffuse = vec3(0.f);
    //Only shade dielectrics
    if(shadingData.metalness >= 1) {
        return vec3(0.f);
    }
    if(scene.dirLight.enabled > 0) 
    {
        float lightShadow = light_visibility(accelerationStructure, shadingData, -scene.dirLight.dir, 1e27f);
        if(lightShadow > minLight) {
            LightData lightData;
            lightData.dir = -scene.dirLight.dir;
            lightData.intensity = scene.dirLight.intensity;
            lightData.color = scene.dirLight.color;
            diffuse_light(lightData, shadingData, diffuse);
        }
    }
    if(scene.numPointLights > 0)
    {
        diffuse += diffuse_point_lights(accelerationStructure, scene, shadingData);
    }
    return diffuse;
}

#ifdef SHADOW_RAY_PAYLOAD_TEMPORARY_DEFINE
#undef SHADOW_RAY_PAYLOAD_TEMPORARY_DEFINE
#undef SHADOW_RAY_PAYLOAD_LOCATION
#endif

#endif