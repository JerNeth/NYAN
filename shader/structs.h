#ifndef STRUCTS_H
#define STRUCTS_H

const uint INVALID_BINDING = 4294967295u;
struct Mesh {
	uint materialBinding;
	uint materialId;
	uint64_t indicesAddress;
	uint64_t positionsAddress;
	uint64_t uvsAddress;
	uint64_t normalsAddress;
	uint64_t tangentsAddress;
};

const uint MATERIAL_DOUBLE_SIDED_FLAG = 0x1u << 0;
const uint MATERIAL_ALPHA_TEST_FLAG = 0x1u << 1;
const uint MATERIAL_ALPHA_BLEND_FLAG = 0x1u << 2;

struct Material {
	uint albedoTexId;
	uint albedoSampler;
	uint normalTexId;
	uint normalSampler;

	uint pbrTexId;
	uint pbrSampler;
	uint emissiveTexId;
	uint emissiveSampler;

	float emissive_R;
	float emissive_G;
	float emissive_B;
	float albedo_R;

	float albedo_G;
	float albedo_B;
	float albedo_A;
	float metalness;

	float roughness;
	float anisotropy;
	float alphaDiscard;
	uint flags;
};

struct DirectionalLight {
	vec3 dir;
	float intensity;
	vec3 color;
	uint enabled;
};

struct PointLight {
	vec3 pos;
	float intensity;
	vec3 color;
	float attenuationDistance;
};
const uint maxNumPointLights = 12;

struct SkyLight {
	vec3 color;
	float intensity;
};

struct Scene {
	mat4 view;
	mat4 proj;
	mat4 viewProj;
	mat4 invView;
	mat4 invProj;
	mat4 invViewProj;
	DirectionalLight dirLight;

	uint numPointLights;
	float viewerPosX;
	float viewerPosY;
	float viewerPosZ;

	float cameraUpX;
	float cameraUpY;
	float cameraUpZ;
	float pad0;

	SkyLight skyLight;

	PointLight pointLights[maxNumPointLights];
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
	uint irradianceImageFormat;

	uint depthTextureSizeX;
	uint depthTextureSizeY;
	float inverseDepthTextureSizeX;
	float inverseDepthTextureSizeY;
	uint depthTextureBinding;
	uint depthTextureSampler;
	uint depthImageBinding;
	uint depthImageFormat;

	uint renderTargetImageFormat;

	uint offsetBufferBinding;
	uint fixedRayCount;
	float relocationBackfaceThreshold;
	float minFrontFaceDistance;

	float shadowBias;
	float maxRayDistance;
	float hysteresis;
	float irradianceThreshold;
	float lightToDarkThreshold;

	float visualizerRadius;

	uint enabled;
	uint visualizeDepth;
	uint visualizeDirections;
	uint useMoments;
	uint relocationEnabled;
	uint classificationEnabled;
};

#endif