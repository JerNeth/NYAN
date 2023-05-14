#ifndef BINDLESS_LAYOUTS_GLSL
#define BINDLESS_LAYOUTS_GLSL
#include "descriptors.h"

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer MeshData {
	Mesh meshes[];
} meshData[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer Instances {
	Instance instances[];
} instances[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer Scenes {
	Scene scene;
} scenes[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer DDGIVolumes {
	DDGIVolume volume[];
} ddgiVolumes[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer DDGIReSTIRVolumes {
	DDGIReSTIRVolume volume[];
} ddgiReSTIRVolumes[SSBO_COUNT];

//No real performance gain here
//layout(set = 0, binding = UNIFORM_BUFFER_BINDING, std140) readonly uniform DDGIVolumes {
//	DDGIVolume volume[2];
//} ddgiVolumes[UBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) writeonly buffer WriteDDGIOffsets {
	float offsets[];
} writeDdgiOffsets[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer ReadDDGIOffsets {
	float offsets[];
} readDdgiOffsets[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) writeonly buffer WriteDDGIRayCounts {
	uint offset[];
} writeDdgiRayCounts[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer UInts {
	uint u[];
} uInts[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) writeonly buffer WriteUInts {
	uint u[];
} writeUInts[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer ReadUInts {
	uint u[];
} readUInts[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer Floats {
	float f[];
} floats[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) writeonly buffer WriteFloats {
	float f[];
} writeFloats[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer ReadFloats {
	float f[];
} readFloats[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer ReadDDGIRayCounts {
	uint offset[];
} readDdgiRayCounts[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) writeonly buffer WriteDDGIRayVariance {
	float sigma[];
} writeDdgiRayVariance[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer ReadDDGIRayVariance {
	float sigma[];
} readDdgiRayVariance[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) readonly buffer Materials  {
	Material materials[];
} materials [SSBO_COUNT];

layout(set = 0, binding = SAMPLER_BINDING) uniform sampler samplers[SAMPLER_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform texture2D textures2D[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform itexture2D itextures2D[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform utexture2D utextures2D[TEXTURE_COUNT];

layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform texture2DArray textures2DArray[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform itexture2DArray itextures2DArray[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform utexture2DArray utextures2DArray[TEXTURE_COUNT];

layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgb10_a2) uniform image2D imagesRGB10_A2[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r11f_g11f_b10f) uniform image2D imagesR11G11B10F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rg16f) uniform image2D imagesRG16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgba16f) uniform image2D imagesRGBA16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgba32f) uniform image2D imagesRGBA32F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r32f) uniform image2D imagesR32F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r32ui) uniform uimage2D imagesR32UI[IMAGE_COUNT];

layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgb10_a2) uniform image2DArray imagesArrayRGB10_A2[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r11f_g11f_b10f) uniform image2DArray imagesArrayR11G11B10F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rg16f) uniform image2DArray imagesArrayRG16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rg16f) readonly uniform image2DArray imagesArrayReadRG16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rg16f) writeonly uniform image2DArray imagesArrayWriteRG16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgba16f) uniform image2DArray imagesArrayRGBA16F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgba32f) uniform image2DArray imagesArrayRGBA32F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r32f) uniform image2DArray imagesArrayR32F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r32ui) uniform uimage2DArray imagesArrayR32UI[IMAGE_COUNT];

//layout(set = 0, binding = 0) buffer SSBO {} ssbos [];
//layout(set = 0, binding = 1) uniform ubos[];
//layout(set = 0, binding = 2) uniform sampler samplers[256];
//layout(set = 0, binding = 3) uniform texture2D textures[512 * 1024];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];
#endif