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
		.mipLevels = info.mipLevels,
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
	if (m_vmaAllocation != VK_NULL_HANDLE) {
		vmaDestroyImage(r_device.get_vma_allocator(), m_vkHandle, m_vmaAllocation);
	}
}
