#include "Image.h"
#include "LogicalDevice.h"
#include "Allocator.h"

vulkan::ImageView::ImageView(LogicalDevice& parent, const ImageViewCreateInfo& info) :
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
			Utility::log_error().location().format("VK: error %d while creating ImageView", static_cast<int>(result));
			throw std::runtime_error("VK: error");
		}
	}
}


vulkan::ImageView::ImageView(ImageView&& other) noexcept:
    r_device(other.r_device),
    m_vkHandle(other.m_vkHandle)
{
    other.m_vkHandle = VK_NULL_HANDLE;
}
vulkan::ImageView::~ImageView() noexcept {
    if(m_vkHandle != VK_NULL_HANDLE) {
        r_device.queue_image_view_deletion(m_vkHandle);
    }
}

vulkan::Image::Image(LogicalDevice& parent, VkImage image, const ImageInfo& info, const std::vector< AllocationHandle>& allocations, uint32_t mipTail) :
	r_device(parent),
	m_vkHandle(image),
	m_info(info),
	m_allocations(allocations),
	m_view(r_device.create_image_view(ImageViewCreateInfo{
		.image = this,
		.format = info.format,
		.viewType = info.view_type(),
		.aspectMask = ImageInfo::format_to_aspect_mask(info.format),
		.baseMipLevel = mipTail,
		.levelCount = info.mipLevels - mipTail,
		.baseArrayLayer = 0,
		.layerCount = info.arrayLayers,
		})),
	m_availableMip(mipTail),
	m_mipTail(mipTail)
{
}

vulkan::Image::Image(Image&& other) noexcept :
	r_device(other.r_device),
	m_optimal(other.m_optimal),
	m_ownsImage(other.m_ownsImage),
	m_vkHandle(other.m_vkHandle),
	m_stageFlags(other.m_stageFlags),
	m_accessFlags(other.m_accessFlags),
	m_info(other.m_info),
	m_allocations(std::move(other.m_allocations)),
	m_view(std::move(other.m_view)),
	m_availableMip(other.m_availableMip),
	m_mipTail(other.m_mipTail),
	m_singleMipTail(other.m_singleMipTail),
	m_isBeingResized(other.m_isBeingResized)
{
	other.m_vkHandle = VK_NULL_HANDLE;
}

vulkan::Image& vulkan::Image::operator=(Image&& other) noexcept
{
	if (this != &other) {
		if (m_ownsImage)
			if (m_vkHandle != VK_NULL_HANDLE)
				r_device.queue_image_deletion(m_vkHandle);
		m_allocations.clear();
		m_optimal = other.m_optimal;
		m_ownsImage = other.m_ownsImage;
		m_vkHandle = other.m_vkHandle;
		m_stageFlags = other.m_stageFlags;
		m_accessFlags = other.m_accessFlags;
		m_info = other.m_info;
		m_allocations = std::move(other.m_allocations);
		m_view = std::move(other.m_view);
		m_availableMip = other.m_availableMip;
		m_mipTail = other.m_mipTail;
		m_singleMipTail = other.m_singleMipTail;
		m_isBeingResized = other.m_isBeingResized;
		other.m_vkHandle = VK_NULL_HANDLE;
	}
	return *this;
}

vulkan::Image::~Image() noexcept
{		
	if (m_ownsImage)
		if (m_vkHandle != VK_NULL_HANDLE)
			r_device.queue_image_deletion(m_vkHandle);
	
}

void vulkan::Image::append_allocations(const std::vector<AllocationHandle>& allocations)
{
	m_allocations.reserve(m_allocations.size() + allocations.size());
	m_allocations.insert(m_allocations.end(), allocations.cbegin(), allocations.cend());
}

void vulkan::Image::drop_allocations(uint32_t count)
{
	uint32_t releaseCount = Math::min(count, static_cast<uint32_t>(m_allocations.size()));
	m_allocations.erase(m_allocations.end() - releaseCount,m_allocations.end());
}

vulkan::ImageView* vulkan::Image::change_view_mip_level(uint32_t mip) {
	if (mip == m_view->get_base_mip_level())
		return nullptr;
	if (mip >= m_info.mipLevels)
		return nullptr;
	ImageViewCreateInfo createInfo;
	createInfo.image = this;
	createInfo.viewType = m_info.view_type();
	createInfo.format = m_info.format;
	createInfo.baseMipLevel = mip;
	createInfo.levelCount = m_info.mipLevels - mip;
	createInfo.baseArrayLayer = 0;
	createInfo.layerCount = m_info.arrayLayers;
	createInfo.aspectMask = ImageInfo::format_to_aspect_mask(m_info.format);
	m_view = r_device.create_image_view(createInfo);
	return &(*m_view);
}