#ifndef VKIMAGE_H
#define VKIMAGE_H
#pragma once
#include "VulkanIncludes.h"
#include "LinAlg.h"

namespace Vulkan {
	class LogicalDevice;
	class Image;
	class ImageView;
	
	struct ImageInfo {
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
		bool isSwapchainImage = false;
		uint32_t width = 0;
		uint32_t height = 0;
		uint32_t depth = 1;
		uint32_t mipLevels = 1;
		uint32_t arrayLayers = 1;
		VkImageUsageFlags usage = 0;
		VkImageType type = VK_IMAGE_TYPE_2D;
		VkImageLayout layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		static bool is_depth_or_stencil_format(VkFormat format) {
			switch (format) {
			case(VK_FORMAT_D16_UNORM):
			case(VK_FORMAT_D16_UNORM_S8_UINT):
			case(VK_FORMAT_D24_UNORM_S8_UINT):
			case(VK_FORMAT_S8_UINT):
			case(VK_FORMAT_D32_SFLOAT):
			case(VK_FORMAT_D32_SFLOAT_S8_UINT):
			case VK_FORMAT_X8_D24_UNORM_PACK32:
				return true;

			default:
				return false;
			}
		}
		static ImageInfo render_target(uint32_t width, uint32_t height, VkFormat format) {
			ImageInfo info;
			info.width = width;
			info.height = height;
			info.depth = 1;
			info.mipLevels = 1;
			info.arrayLayers = 1;
			info.type = VK_IMAGE_TYPE_2D;
			if (is_depth_or_stencil_format(format))
				info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			else
				info.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			info.usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
			info.layout = VK_IMAGE_LAYOUT_UNDEFINED;
			return info;
		}
		static uint32_t compute_view_formats(const ImageInfo& info, std::array<VkFormat, 2>& formats) {
			switch (info.format) {
			case VK_FORMAT_R8G8B8A8_UNORM:
			case VK_FORMAT_R8G8B8A8_SRGB:
				formats[0] = VK_FORMAT_R8G8B8A8_UNORM;
				formats[1] = VK_FORMAT_R8G8B8A8_SRGB;
				return 2;

			case VK_FORMAT_B8G8R8A8_UNORM:
			case VK_FORMAT_B8G8R8A8_SRGB:
				formats[0] = VK_FORMAT_B8G8R8A8_UNORM;
				formats[1] = VK_FORMAT_B8G8R8A8_SRGB;
				return 2;

			case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
				formats[0] = VK_FORMAT_A8B8G8R8_UNORM_PACK32;
				formats[1] = VK_FORMAT_A8B8G8R8_SRGB_PACK32;
				return 2;

			default:
				return 0;
			}
		}
	};
	
	
	struct ImageViewCreateInfo {
		Image* image = nullptr;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkComponentMapping swizzle{
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY,
				.b = VK_COMPONENT_SWIZZLE_IDENTITY,
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
		};
		VkImageAspectFlags aspectMask = 0;
		uint32_t baseMipLevel = 0;
		uint32_t levelCount = 1;
		uint32_t baseArrayLayer = 0;
		uint32_t layerCount = 1;
	};
	class ImageView {
	public:
		ImageView(LogicalDevice& parent, const ImageViewCreateInfo & info);
		ImageView(ImageView& other) = delete;
		ImageView(ImageView&& other) noexcept;
		ImageView& operator=(ImageView&) = delete;
		ImageView& operator=(ImageView&&) = delete;
		~ImageView() noexcept;
		VkImageView get_image_view() const noexcept {
			return m_vkHandle;
		}
		VkFormat get_format() const noexcept {
			return m_info.format;
		}
		Image* get_image() const noexcept {
			return m_info.image;
		}
		uint32_t get_base_mip_level() const noexcept {
			return m_info.baseMipLevel;
		}
	private:
		LogicalDevice& r_device;
		VkImageView m_vkHandle;
		ImageViewCreateInfo m_info;
	};
	class Image {
	public:
		Image(LogicalDevice& parent, VkImage image,const ImageInfo& info);
		Image(LogicalDevice& parent,const ImageInfo& info, VmaMemoryUsage usage);
		Image(Image&) = delete;
		Image(Image&&) noexcept;
		Image& operator=(Image&) = delete;
		Image& operator=(Image&&) = delete;
		~Image() noexcept;
		uint32_t get_width(uint32_t mipLevel = 0) const noexcept {
			return Math::max(1u, m_info.width >> mipLevel);
		}
		uint32_t get_height(uint32_t mipLevel = 0) const noexcept {
			return Math::max(1u, m_info.height >> mipLevel);
		}
		VkImage get_handle() const noexcept {
			return m_vkHandle;
		}
		VkImageUsageFlags get_usage() const noexcept {
			return m_info.usage;
		}
		const ImageInfo& get_info() const {
			return m_info;
		}
	private:
		LogicalDevice& r_device;
		VkImage m_vkHandle = VK_NULL_HANDLE;
		VmaAllocation m_vmaAllocation = VK_NULL_HANDLE;
		ImageInfo m_info;
	};
}

#endif //VKIMAGE_H