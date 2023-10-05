#ifndef DESCRIPTORS_H
#define DESCRIPTORS_H

const int SSBO_COUNT = 1 * 1024;
const int UBO_COUNT = 15;
const int SAMPLER_COUNT = 256;
const int TEXTURE_COUNT = 8 * 1024;
const int IMAGE_COUNT = 8 * 1024;
const int ACC_COUNT = 256;

const int STORAGE_BUFFER_BINDING = 0;
const int UNIFORM_BUFFER_BINDING = 1;
const int SAMPLER_BINDING = 2;
const int SAMPLED_IMAGE_BINDING = 3;
const int STORAGE_IMAGE_BINDING = 4;
const int ACCELERATION_STRUCTURE_BINDING = 5;

#ifdef __cplusplus
namespace vulkan {
	static constexpr inline uint32_t storageBufferBinding = STORAGE_BUFFER_BINDING;
	static constexpr inline uint32_t uniformBufferBinding = UNIFORM_BUFFER_BINDING;
	static constexpr inline uint32_t samplerBinding = SAMPLER_BINDING;
	static constexpr inline uint32_t sampledImageBinding = SAMPLED_IMAGE_BINDING;
	static constexpr inline uint32_t storageImageBinding = STORAGE_IMAGE_BINDING;
	static constexpr inline uint32_t accelerationStructureBinding = ACCELERATION_STRUCTURE_BINDING;
};
#endif
#endif