module;

#include <cassert>
#include <expected>
#include <utility>
#include <source_location>
#include <span>

#include "magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"


module NYANVulkan;

import NYANLog;
import :LogicalDevice;
import :PhysicalDevice;

using namespace nyan::vulkan;
using namespace nyan;


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
			log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension1D);
		if (createInfo.extent.height != 1)
			log::error().location(location).format("Requested image height \"{}\" doesn't match 1D Image type",
				createInfo.extent.height);
		if (createInfo.extent.depth != 1)
			log::error().location(location).format("Requested image depth \"{}\" doesn't match 1D Image type",
				createInfo.extent.depth);
		assert(createInfo.extent.width <=limits.maxImageDimension1D);
		assert(createInfo.extent.height == 1);
		assert(createInfo.extent.depth == 1);
		assert(createInfo.arrayLayers <= limits.maxImageArrayLayers);
	} else if (createInfo.imageType == VK_IMAGE_TYPE_2D) {
		if (createInfo.extent.width > limits.maxImageDimension2D)
			log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension2D);
		if (createInfo.extent.height > limits.maxImageDimension2D)
			log::error().location(location).format("Requested image height \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.height, limits.maxImageDimension2D);
		if (createInfo.extent.depth != 1)
			log::error().location(location).format("Requested image depth \"{}\" doesn't match 2D Image type",
				createInfo.extent.depth);
		assert(createInfo.extent.width <= limits.maxImageDimension2D);
		assert(createInfo.extent.height <= limits.maxImageDimension2D);
		assert(createInfo.extent.depth == 1);
		assert(createInfo.arrayLayers <= limits.maxImageArrayLayers);
	} else if (createInfo.imageType == VK_IMAGE_TYPE_3D) {
		if (createInfo.extent.width > limits.maxImageDimension3D)
			log::error().location(location).format("Requested image width \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.width, limits.maxImageDimension3D);
		if (createInfo.extent.height > limits.maxImageDimension3D)
			log::error().location(location).format("Requested image height \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.height, limits.maxImageDimension3D);
		if (createInfo.extent.depth > limits.maxImageDimension3D)
			log::error().location(location).format("Requested image depth \"{}\" exceeds device limits \"{}\"",
				createInfo.extent.depth, limits.maxImageDimension3D);
		assert(createInfo.extent.width <= limits.maxImageDimension3D);
		assert(createInfo.extent.height <= limits.maxImageDimension3D);
		assert(createInfo.extent.depth <= limits.maxImageDimension3D);
		assert(createInfo.arrayLayers == 1);
	}
	return true;
}

[[nodiscard]] constexpr static VkImageViewType get_view_type(Image::Type type)
{
	switch (type) {
		using enum Image::Type;
	case Texture:
	case Staging:
	case RenderTarget:
	case Swapchain:
		return VK_IMAGE_VIEW_TYPE_2D;
	default:
		std::unreachable(); //UB, should never happen since the switch should cover all enums
	}
}

Image::Image(Image&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	r_allocator(other.r_allocator),
	m_data(std::exchange(other.m_data, {}))
{
}

Image& Image::operator=(Image&& other) noexcept
{
	assert(ptr_device == other.ptr_device);
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
	for(const auto& view : m_data.views)
		r_deletionQueue.queue_deletion(view.get_handle());
	if (m_handle != VK_NULL_HANDLE && m_data.type != Type::Swapchain)
		r_deletionQueue.queue_deletion(m_handle);
	if (m_data.allocation != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_data.allocation);
}

uint16_t Image::get_width() const noexcept
{
	return m_data.width;
}

uint16_t Image::get_height() const noexcept
{
	return m_data.height;
}

Queue::FamilyIndex::Group Image::get_queue_families() const noexcept
{
	return m_data.queueFamilies;
}

uint16_t Image::get_layers() const noexcept
{
	return m_data.layers;
}

uint16_t Image::get_mip_levels() const noexcept
{
	return m_data.mipLevels;
}

Samples Image::get_samples() const noexcept
{
	return m_data.samples;
}

Image::Type Image::get_type() const noexcept
{
	return m_data.type;
}

const ImageView& Image::get_image_view(size_t idx) const noexcept
{
	assert(idx < m_data.views.size());
	assert(idx < m_data.views.capacity());
	return m_data.views[idx];
}

Format Image::get_format() const noexcept
{
	return m_data.format;
}
ImageUsage Image::get_usage() const noexcept
{
	return m_data.usage;
}

Image::Image(Parameters parameters) noexcept :
	Object(parameters.device, parameters.handle),
	r_deletionQueue(parameters.deletionQueue),
	r_allocator(parameters.allocator),
	m_data(std::move(parameters.data))
{
	assert(m_handle != VK_NULL_HANDLE);
}

std::expected<RenderTarget, Error> RenderTarget::create(LogicalDevice& device, RenderTargetOptions options) noexcept
{
	const auto& physicalDevice = device.get_physical_device();

	std::array<uint32_t, Queue::FamilyIndex::max> queueFamilies{};

	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.flags = 0,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = static_cast<VkFormat>(options.format),
		.extent {
			.width = options.width,
			.height = options.height,
			.depth = 1
		},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = static_cast<VkSampleCountFlagBits>( options.samples),
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = static_cast<VkImageUsageFlags>( convert_image_usage(options.options.usage)),
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = queueFamilies.data(),
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	validate_image_create_info(physicalDevice.get_properties().limits, createInfo);
	if (!physicalDevice.supports_image_format(createInfo.format, createInfo.imageType, createInfo.tiling, createInfo.usage, createInfo.flags)) [[unlikely]]
		return std::unexpected{ VK_ERROR_FORMAT_NOT_SUPPORTED };

	if (options.options.queueFamilies.size() > 1) {
		options.options.queueFamilies.fill(std::span{ queueFamilies.data(), options.options.queueFamilies.size() });
		createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = options.options.queueFamilies.size();
	}

	VmaAllocationCreateInfo allocationCreateInfo
	{
		.flags {},
		.usage {VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
	};

	if (options.dedicatedAllocation)
		allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	auto allocatorHandle = device.get_allocator().get_handle();

	VkImage image{ VK_NULL_HANDLE };
	VmaAllocation allocation{ VK_NULL_HANDLE };
	VmaAllocationInfo allocationInfo;
	if (auto result = vmaCreateImage(allocatorHandle, &createInfo, &allocationCreateInfo, &image, &allocation, &allocationInfo); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	VkMemoryPropertyFlags memPropFlags;
	vmaGetAllocationMemoryProperties(allocatorHandle, allocation, &memPropFlags);


	assert(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (!(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) [[unlikely]] {
		vmaDestroyImage(allocatorHandle, image, allocation);
		return std::unexpected{ VK_ERROR_OUT_OF_DEVICE_MEMORY };
	}


	decltype(Image::Data::views) views;

	auto type = Type::RenderTarget;
	if (auto ret = views.try_push_back(ImageView::create(device, ImageView::Params{ .image {image}, .format {static_cast<Format>(options.format)}, .type {get_view_type(type)} }));
		!ret.has_value() || !ret.value()) [[unlikely]] {
		vmaDestroyImage(allocatorHandle, image, allocation);
		return std::unexpected{ ret.error() };
	}
	
	

	return RenderTarget{ Parameters {device.get_device(), device.get_deletion_queue(), device.get_allocator(), image, Data
			{
				.usage {options.options.usage},
				.location {Location::Device},
				.queueFamilies{options.options.queueFamilies},
				.allocation{ allocation },
				.width {static_cast<uint16_t>(createInfo.extent.width)},
				.height {static_cast<uint16_t>(createInfo.extent.height)},
				.layers {static_cast<uint16_t>(createInfo.arrayLayers)},
				.mipLevels {static_cast<uint16_t>(createInfo.mipLevels)},
				.samples {options.samples},
				.type {type},
				.format {static_cast<Format>(options.format)},
				.views {std::move(views)}
			}
		}
	};
}

RenderTarget::RenderTarget(Parameters parameters) noexcept :
	Image(std::move(parameters))
{

}

std::expected<SwapchainImage, Error> SwapchainImage::create(LogicalDevice& device, SwapchainImageOptions options) noexcept
{

	decltype(Image::Data::views) views;

	auto type = Type::Swapchain;
	if (auto ret = views.try_push_back(ImageView::create(device, ImageView::Params{ .image {options.handle}, .format {static_cast<Format>(options.format)}, .type {get_view_type(type)} }));
		!ret.has_value() || !ret.value()) [[unlikely]]
		return std::unexpected{ ret.error() };

	return SwapchainImage{ Parameters {device.get_device(), device.get_deletion_queue(), device.get_allocator(), options.handle, Data
			{
				.usage {options.options.usage},
				.location {Location::Device},
				.queueFamilies{options.options.queueFamilies},
				.allocation{ VK_NULL_HANDLE },
				.width {static_cast<uint16_t>(options.width)},
				.height {static_cast<uint16_t>(options.height)},
				.layers {static_cast<uint16_t>(1)},
				.mipLevels {static_cast<uint16_t>(1)},
				.samples {Samples::C1},
				.type {type},
				.format {static_cast<Format>(options.format)},
				.views {std::move(views)}
			}
		}
	};
}

SwapchainImage::SwapchainImage(Parameters parameters) noexcept :
	RenderTarget(std::move(parameters))
{

}