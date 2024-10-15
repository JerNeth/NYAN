module;

//#include <cassert>
//#include <expected>
//#include <utility>
//#include <source_location>

#include "magic_enum/magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"


module NYANVulkan;
import std;

import NYANLog;
import :LogicalDevice;
import :PhysicalDevice;
import :Image;

using namespace nyan::vulkan;


[[nodiscard]] constexpr static VkImageAspectFlagBits convert_image_view_aspect_flags(ImageView::AspectFlags aspect)
{
	switch (aspect) {
		using enum ImageView::AspectFlags;
	case Color:
		return VK_IMAGE_ASPECT_COLOR_BIT;
	case Depth:
		return VK_IMAGE_ASPECT_DEPTH_BIT;
	case Stencil:
		return VK_IMAGE_ASPECT_STENCIL_BIT;
	default:
		std::unreachable(); //UB, should never happen since the switch should cover all enums
	}
}

[[nodiscard]] constexpr static VkImageAspectFlags convert_aspect(ImageView::Aspect aspect)
{
	VkImageAspectFlags aspectFlags{};
	aspect.for_each([&aspectFlags](ImageView::AspectFlags aspect) {aspectFlags |= convert_image_view_aspect_flags(aspect); });
	return aspectFlags;
}


std::expected<ImageView, Error> nyan::vulkan::ImageView::create(LogicalDevice& device, Params params) noexcept
{
	if (params.format == Format::UNDEFINED) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	if (params.aspect.test(AspectFlags::Color) && params.aspect.any_of(AspectFlags::Depth, AspectFlags::Stencil)) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	VkImageViewCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.image {params.image},
		.viewType {params.type},
		.format {static_cast<VkFormat>(params.format)},
		.components {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY}, //Must be identity for basically anything but sampled images (Simplified)
		.subresourceRange {
			.aspectMask {convert_aspect(params.aspect)},
			.baseMipLevel {params.level},
			.levelCount {params.levelCount},
			.baseArrayLayer {params.layer},
			.layerCount {params.layerCount},
		},
	};
	VkImageView view{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateImageView(&createInfo, &view); result != VK_SUCCESS) [[unlikely]] {
		return std::unexpected{ result };
	}
	return ImageView{ view };
}

VkImageView nyan::vulkan::ImageView::get_handle() const noexcept
{
	return m_handle;
}

ImageView::ImageView(VkImageView handle) noexcept :
	m_handle(handle)
{

}
