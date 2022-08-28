#ifndef STRUCTS_H
#define STRUCTS_H

struct Mesh {
	uint materialBinding;
	uint materialId;
	uint64_t indicesAddress;
	uint64_t positionsAddress;
	uint64_t uvsAddress;
	uint64_t normalsAddress;
	uint64_t tangentsAddress;
};

struct Material {
	uint albedoTexId;
	uint albedoSampler;
	uint normalTexId;
	uint normalSampler;
	uint pbrTexId;
	uint pbrSampler;
	float albedo_R;
	float albedo_G;
	float albedo_B;
	float albedo_A;
	float metalness;
	float roughness;
	float anisotropy;
	//float IoR;
	//float F0_R;
	//float F0_G;
	//float F0_B;
	float alphaDiscard;

};

struct DirectionalLight {
	vec3 dir;
	float intensity;
	vec3 color;
	bool enabled;
};

struct PointLight {
	vec3 pos;
	float intensity;
	vec3 color;
	float attenuationDistance;
};
struct Scene {
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 invView;
	mat4 invProj;
	mat4 invViewProj;
	DirectionalLight dirLight;
	uint numLights;
	float viewerPosX;
	float viewerPosY;
	float viewerPosZ;
	PointLight pointLights[256];
};

struct Instance {
	vec4 modelRow1;
	vec4 modelRow2;
	vec4 modelRow3;
	uint meshId;
	uint pad1;
	uint pad2;
	uint pad3;
};

struct DDGIVolume {
	//Assuming Grid
	float spacingX;
	float spacingY;
	float spacingZ;

	float inverseSpacingX;
	float inverseSpacingY;
	float inverseSpacingZ;

	float gridOriginX;
	float gridOriginY;
	float gridOriginZ;

	uint probeCountX;
	uint probeCountY;
	uint probeCountZ;

	uint raysPerProbe;

	//Probe size in px 
	uint irradianceProbeSize;
	uint depthProbeSize;

	uint irradianceTextureSizeX;
	uint irradianceTextureSizeY;
	float inverseIrradianceTextureSizeX;
	float inverseIrradianceTextureSizeY;
	uint irradianceTextureBinding;
	uint irradianceTextureSampler;
	uint irradianceImageBinding;

	uint depthTextureSizeX;
	uint depthTextureSizeY;
	float inverseDepthTextureSizeX;
	float inverseDepthTextureSizeY;
	uint depthTextureBinding;
	uint depthTextureSampler;
	uint depthImageBinding;

	float shadowBias;
	float maxRayDistance;
};

#endif