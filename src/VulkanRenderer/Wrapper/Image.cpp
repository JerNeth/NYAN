module;

#include <cassert>
#include <iostream>
#include <utility>
#include <source_location>

#include "magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"


module NYANVulkanWrapper;

import NYANLog;
import :LogicalDevice;

using namespace nyan::vulkan::wrapper;


static bool validate_image_create_info(const VkPhysicalDeviceLimits& limits, const VkImageCreateInfo& createInfo, std::source_location location = std::source_location::current()) {
	assert(createInfo.extent.width > 0);
	assert(createInfo.extent.height > 0);
	assert(createInfo.extent.depth > 0);
	assert(createInfo.arrayLayers > 0);
	assert(createInfo.mipLevels > 0);

	if (createInfo.tiling == VK_IMAGE_TILING_LINEAR) {
		assert(createInfo.mipLevels == 1);
		assert(createInfo.arrayLayers == 1);
		assert(createInfo.samples & VK_SAMPLE_COUNT_1_BIT);
		assert(!(createInfo.usage & ~(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)));
	}

	if (createInfo.imageType == VK_IMAGE_TYPE_1D) {
		if (createInfo.extent.width > limits.maxImageDimension1D)
			nyan::util::log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension1D);
		if (createInfo.extent.height != 1)
			nyan::util::log::error().location(location).format("Requested image height \"{}\" doesn't match 1D Image type",
				createInfo.extent.height);
		if (createInfo.extent.depth != 1)
			nyan::util::log::error().location(location).format("Requested image depth \"{}\" doesn't match 1D Image type",
				createInfo.extent.depth);
		assert(createInfo.extent.width <=limits.maxImageDimension1D);
		assert(createInfo.extent.height == 1);
		assert(createInfo.extent.depth == 1);
		assert(createInfo.arrayLayers <= limits.maxImageArrayLayers);
	}

	if (createInfo.imageType == VK_IMAGE_TYPE_2D) {
		if (createInfo.extent.width > limits.maxImageDimension2D)
			nyan::util::log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension2D);
		if (createInfo.extent.height > limits.maxImageDimension2D)
			nyan::util::log::error().location(location).format("Requested image height \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.height, limits.maxImageDimension2D);
		if (createInfo.extent.depth != 1)
			nyan::util::log::error().location(location).format("Requested image depth \"{}\" doesn't match 2D Image type",
				createInfo.extent.depth);
		assert(createInfo.extent.width <= limits.maxImageDimension2D);
		assert(createInfo.extent.height <= limits.maxImageDimension2D);
		assert(createInfo.extent.depth == 1);
		assert(createInfo.arrayLayers <= limits.maxImageArrayLayers);
	}
	if (createInfo.imageType == VK_IMAGE_TYPE_3D) {
		if (createInfo.extent.width > limits.maxImageDimension3D)
			nyan::util::log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension3D);
		if (createInfo.extent.height > limits.maxImageDimension3D)
			nyan::util::log::error().location(location).format("Requested image height \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.height, limits.maxImageDimension3D);
		if (createInfo.extent.depth > limits.maxImageDimension3D)
			nyan::util::log::error().location(location).format("Requested image depth \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.depth, limits.maxImageDimension3D);
		assert(createInfo.extent.width <= limits.maxImageDimension3D);
		assert(createInfo.extent.height <= limits.maxImageDimension3D);
		assert(createInfo.extent.depth <= limits.maxImageDimension3D);
		assert(createInfo.arrayLayers == 1);
	}
}

[[nodiscard]] VkImageUsageFlagBits convert_image_usage_flags(Image::UsageFlags usage) 
{
	switch (usage) {
		using enum Image::UsageFlags;
		case TransferSrc: 
			return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case TransferDst: 
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case Sampled: 
			return VK_IMAGE_USAGE_SAMPLED_BIT;
		case Storage: 
			return VK_IMAGE_USAGE_STORAGE_BIT;
		case ColorAttachment: 
			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case DepthStencilAttachment: 
			return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case TransientAttachment: 
			return VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		case InputAttachment: 
			return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		case VideoDecodeDst: 
			return VK_IMAGE_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		case VideoDecodeSrc: 
			return VK_IMAGE_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		case VideoDecodeDPB: 
			return VK_IMAGE_USAGE_VIDEO_DECODE_DPB_BIT_KHR;
		case FragmentDensityMap: 
			return VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
		case FragmentShadingRateAttachment: 
			return VK_IMAGE_USAGE_FRAGMENT_SHADING_RATE_ATTACHMENT_BIT_KHR;
		case HostTransfer: 
			return VK_IMAGE_USAGE_HOST_TRANSFER_BIT_EXT;
		case VideoEncodeDst: 
			return VK_IMAGE_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
		case VideoEncodeSrc: 
			return VK_IMAGE_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
		case VideoEncodeDPB: 
			return VK_IMAGE_USAGE_VIDEO_ENCODE_DPB_BIT_KHR;
		default:
			std::unreachable(); //UB, should never happen since the switch should cover all enums
	}
}

[[nodiscard]] VkImageUsageFlags convert_image_usage(Image::Usage usage)
{
	VkImageUsageFlags usageFlags{};
	usage.for_each([&usageFlags](Image::UsageFlags usage) {usageFlags |= convert_image_usage_flags(usage); });
	return usageFlags;
}


Image::Image(Image&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	r_allocator(other.r_allocator),
	m_data(other.m_data)
{
}

Image& Image::operator=(Image&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	assert(std::addressof(r_allocator) == std::addressof(other.r_allocator));
	assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	if (this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_data, other.m_data);
	}
	return *this;
}

Image::~Image() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_image_deletion(m_handle);
	if (m_data.allocation != VK_NULL_HANDLE)
		r_deletionQueue.queue_allocation_deletion(m_data.allocation);
}

Image::Image(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkImage handle, Data data) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	r_allocator(allocator),
	m_data(std::move(data))
{
	assert(m_handle != VK_NULL_HANDLE);
}