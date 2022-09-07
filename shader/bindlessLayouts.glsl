#ifndef BINDLESS_LAYOUTS_GLSL
#define BINDLESS_LAYOUTS_GLSL
#include "descriptors.h"

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer MeshData {
	Mesh meshes[];
} meshData[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer Instances {
	Instance instances[];
} instances[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer Scenes {
	Scene scene;
} scenes[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer DDGIVolumes {
	DDGIVolume volume[];
} ddgiVolumes[SSBO_COUNT];

layout(set = 0, binding = STORAGE_BUFFER_BINDING, std430) buffer Materials  {
	Material materials[];
} materials [SSBO_COUNT];

layout(set = 0, binding = SAMPLER_BINDING) uniform sampler samplers[SAMPLER_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform texture2D textures2D[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform itexture2D itextures2D[TEXTURE_COUNT];
layout(set = 0, binding = SAMPLED_IMAGE_BINDING) uniform utexture2D utextures2D[TEXTURE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgb10_a2) uniform image2D imagesRGB10_A2[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, r11f_g11f_b10f) uniform image2D imagesR11G11B10F[IMAGE_COUNT];
layout(set = 0, binding = STORAGE_IMAGE_BINDING, rgba16f) uniform image2D imagesRGBA16F[IMAGE_COUNT];

//layout(set = 0, binding = 0) buffer SSBO {} ssbos [];
//layout(set = 0, binding = 1) uniform ubos[];
//layout(set = 0, binding = 2) uniform sampler samplers[256];
//layout(set = 0, binding = 3) uniform texture2D textures[512 * 1024];
//layout(set = 0, binding = 4) uniform image2D images[];
//layout(set = 0, binding = 5) uniform accelerationStructureEXT accelerationStructures[];
#endif