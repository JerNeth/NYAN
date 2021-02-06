#include "Image.h"
#include "LogicalDevice.h"

Vulkan::ImageView::ImageView(LogicalDevice& parent, const ImageViewCreateInfo& info) :
	r_device(parent),
	m_info(info)
{
	VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_info.image->get_handle(),
			.viewType = m_info.viewType,
			.format = m_info.format,
			.components = m_info.swizzle,
			.subresourceRange{
				.aspectMask = m_info.aspectMask,
				.baseMipLevel = m_info.baseMipLevel,
				.levelCount = m_info.levelCount,
				.baseArrayLayer = m_info.baseArrayLayer,
				.layerCount = m_info.layerCount
			}
	};
	if (auto result = vkCreateImageView(r_device.get_device(), &createInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create image view, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create image view, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}


Vulkan::ImageView::ImageView(ImageView&& other) noexcept:
    r_device(other.r_device),
    m_vkHandle(other.m_vkHandle)
{
    other.m_vkHandle = VK_NULL_HANDLE;
}
Vulkan::ImageView::~ImageView() noexcept {
    if(m_vkHandle != VK_NULL_HANDLE) {
        vkDestroyImageView(r_device.get_device(), m_vkHandle, r_device.get_allocator());
    }
}

Vulkan::Image::Image(LogicalDevice& parent, VkImage image, const ImageInfo& info) :
	r_device(parent),
	m_vkHandle(image),
	m_info(info)
{
}
Vulkan::Image::Image(LogicalDevice& parent, const ImageInfo& info, VmaMemoryUsage usage) :
	r_device(parent),
	m_info(info)
{
	VkImageCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = info.type,
		.format = info.format,
		.extent {
			.width = info.width,
			.height = info.height,
			.depth = info.depth
		},
		.mipLevels = info.mipLevels == 0? calculate_mip_levels(info.width, info.height, info.depth):info.mipLevels,
		.arrayLayers = info.arrayLayers,
		.samples = info.samples,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = info.usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = info.layout,
	};

	VmaAllocationCreateInfo allocInfo{
		.usage = usage,
	};

	if (auto result = vmaCreateImage(r_device.get_vma_allocator(), &createInfo, &allocInfo, &m_vkHandle, &m_vmaAllocation, nullptr); result != VK_SUCCESS) {
		throw std::runtime_error("Vk: error creating image");
	}
	
}

Vulkan::Image::Image(Image&& other) noexcept :
	r_device(other.r_device),
	m_vkHandle(other.m_vkHandle),
	m_vmaAllocation(other.m_vmaAllocation),
	m_info(other.m_info)
{
	other.m_vmaAllocation = VK_NULL_HANDLE;
	other.m_vkHandle = VK_NULL_HANDLE;
}

Vulkan::Image::~Image() noexcept
{

	
	if (m_ownsAllocation && m_ownsImage) {
		if (m_vmaAllocation != VK_NULL_HANDLE && m_vkHandle != VK_NULL_HANDLE) {
			r_device.queue_image_deletion(m_vkHandle, m_vmaAllocation);
		}
	}
	else if (m_ownsImage) {
		r_device.queue_image_deletion(m_vkHandle);
	}
	else if (m_ownsAllocation) {
		assert(false);
	}
}

uint32_t Vulkan::Image::calculate_mip_levels(uint32_t width, uint32_t height, uint32_t depth)
{
	uint32_t size = Math::max(Math::max(width, height), depth);
	uint32_t mipLevels =0;
	while (size)
	{
		mipLevels++;
		size >>= 1;
	}
	return mipLevels;
}

inline VkPipelineStageFlags Vulkan::Image::possible_stages_from_image_usage(VkImageUsageFlags usage)
{
	VkPipelineStageFlags flags{};
	
	if (usage & (VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT)) {
		flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	if (usage & (VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
	}
	if (usage & (VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
		flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	}
	if (usage & (VK_IMAGE_USAGE_STORAGE_BIT)) {
		flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	if (usage & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT)) {
		flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
		flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT | VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
			VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	}
	return flags;
}

inline VkAccessFlags Vulkan::Image::possible_access_from_image_layout(VkImageLayout layout)
{
	switch (layout) {
	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		return VK_ACCESS_INPUT_ATTACHMENT_READ_BIT | VK_ACCESS_SHADER_READ_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		return VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
		return VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		return VK_ACCESS_TRANSFER_WRITE_BIT;
	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		return VK_ACCESS_TRANSFER_READ_BIT;

	default:
		return ~0u;
	}
}
