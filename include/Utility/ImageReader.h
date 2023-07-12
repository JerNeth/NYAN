#pragma once
#ifndef IMAGEREADER_H
#define IMAGEREADER_H
#include <filesystem>
#include "VulkanWrapper/VulkanIncludes.h"
namespace vulkan { struct ImageInfo; struct InitialImageData; }
namespace Utility {
	struct TextureInfo {
		VkFormat format;
		VkImageType type;
		uint32_t width;
		uint32_t height;
		uint32_t depth;
		uint32_t arrayLayers;
		uint32_t mipLevels;
		bool cube;
	};
	class ImageReader {
	public:
		static std::pair< vulkan::InitialImageData, TextureInfo> read_image_file(const std::filesystem::path& filename);
		static void free_image(const void* img);
	private:
	};
}
#endif //!IMAGEREADER_H