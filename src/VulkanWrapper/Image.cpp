#include "Image.h"
#include "PhysicalDevice.hpp"
#include "LogicalDevice.h"
#include "Allocator.h"
#include "Utility/Exceptions.h"

vulkan::ImageView::ImageView(LogicalDevice& parent, const ImageViewCreateInfo& info) :
	VulkanObject(parent),
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
	if (auto result = r_device.get_device().vkCreateImageView( &createInfo, r_device.get_allocator(), &m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}


vulkan::ImageView::ImageView(ImageView&& other) noexcept:
	VulkanObject(other.r_device, other.m_handle)
{
    other.m_handle = VK_NULL_HANDLE;
}
vulkan::ImageView::~ImageView() noexcept {
    if(m_handle != VK_NULL_HANDLE) {
        r_device.queue_image_view_deletion(m_handle);
    }
}

void vulkan::ImageView::set_debug_label(const char* name) noexcept
{
	if constexpr (debugMarkers) {
		if (r_device.get_physical_device().get_extensions().debug_utils) {
			VkDebugUtilsObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_OBJECT_TYPE_IMAGE_VIEW},
				.objectHandle {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			::vkSetDebugUtilsObjectNameEXT(r_device.get_device_handle(), &label);
		}
		else if (r_device.get_physical_device().get_extensions().debug_marker) {
			//VkDebugMarkerObjectTagInfoEXT label {
			//	.sType {},
			//	.pNext {},
			//	.objectType {},
			//	.object {},
			//	.tagName {},
			//	.tagSize {},
			//	.pTag {},
			//};
			VkDebugMarkerObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT},
				.object {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			vkDebugMarkerSetObjectNameEXT(r_device.get_device_handle(), &label);
		}
	}
}

vulkan::Image::Image(LogicalDevice& parent, VkImage image, const ImageInfo& info, const std::vector< AllocationHandle>& allocations, uint32_t mipTail) :
	VulkanObject(parent, image),
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
	if (ImageInfo::is_stencil(info.format)) {
		m_stencilView = r_device.create_image_view(ImageViewCreateInfo{
			.image = this,
			.format = info.format,
			.viewType = info.view_type(),
			.aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT,
			.baseMipLevel = mipTail,
			.levelCount = info.mipLevels - mipTail,
			.baseArrayLayer = 0,
			.layerCount = info.arrayLayers,
			});
	}
	if (ImageInfo::is_depth(info.format)) {
		m_depthView = r_device.create_image_view(ImageViewCreateInfo{
			.image = this,
			.format = info.format,
			.viewType = info.view_type(),
			.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
			.baseMipLevel = mipTail,
			.levelCount = info.mipLevels - mipTail,
			.baseArrayLayer = 0,
			.layerCount = info.arrayLayers,
			});
	}
}

vulkan::Image::Image(Image&& other) noexcept :
	VulkanObject(other.r_device, other.m_handle),
	m_optimal(other.m_optimal),
	m_ownsImage(other.m_ownsImage),
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
	other.m_handle = VK_NULL_HANDLE;
}

vulkan::Image& vulkan::Image::operator=(Image&& other) noexcept
{
	if (this != &other) {
		if (m_ownsImage)
			if (m_handle != VK_NULL_HANDLE)
				r_device.queue_image_deletion(m_handle);
		m_allocations.clear();
		m_optimal = other.m_optimal;
		m_ownsImage = other.m_ownsImage;
		m_handle = other.m_handle;
		m_stageFlags = other.m_stageFlags;
		m_accessFlags = other.m_accessFlags;
		m_info = other.m_info;
		m_allocations = std::move(other.m_allocations);
		m_view = std::move(other.m_view);
		m_availableMip = other.m_availableMip;
		m_mipTail = other.m_mipTail;
		m_singleMipTail = other.m_singleMipTail;
		m_isBeingResized = other.m_isBeingResized;
		other.m_handle = VK_NULL_HANDLE;
	}
	return *this;
}

vulkan::Image::~Image() noexcept
{		
	if (m_ownsImage)
		if (m_handle != VK_NULL_HANDLE)
			r_device.queue_image_deletion(m_handle);
	
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


vulkan::ImageView* vulkan::Image::get_view() noexcept {
	return &(*m_view);
}
const  vulkan::ImageView* vulkan::Image::get_view() const noexcept {
	return &(*m_view);
}
vulkan::ImageView* vulkan::Image::get_stencil_view() noexcept {
	if (m_stencilView)
		return &(**m_stencilView);
	else
		return nullptr;
}
const  vulkan::ImageView* vulkan::Image::get_stencil_view() const noexcept {
	if (m_stencilView)
		return &(**m_stencilView);
	else
		return nullptr;
}
vulkan::ImageView* vulkan::Image::get_depth_view() noexcept {
	if (m_depthView)
		return &(**m_depthView);
	else
		return nullptr;
}
const  vulkan::ImageView* vulkan::Image::get_depth_view() const noexcept {
	if (m_depthView)
		return &(**m_depthView);
	else
		return nullptr;
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

void vulkan::Image::set_debug_label(const char* name) noexcept
{
	if constexpr (debugMarkers) {
		if (r_device.get_physical_device().get_extensions().debug_utils) {
			VkDebugUtilsObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_OBJECT_TYPE_IMAGE},
				.objectHandle {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			vkSetDebugUtilsObjectNameEXT(r_device.get_device_handle(), &label);
		}
		else if (r_device.get_physical_device().get_extensions().debug_marker) {
			//VkDebugMarkerObjectTagInfoEXT label {
			//	.sType {},
			//	.pNext {},
			//	.objectType {},
			//	.object {},
			//	.tagName {},
			//	.tagSize {},
			//	.pTag {},
			//};
			VkDebugMarkerObjectNameInfoEXT label{
				.sType {VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT},
				.pNext {nullptr},
				.objectType {VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT},
				.object {reinterpret_cast<uint64_t>(m_handle)},
				.pObjectName {name},
			};
			vkDebugMarkerSetObjectNameEXT(r_device.get_device_handle(), &label);
		}
	}
}

uint32_t vulkan::Image::get_available_mip() const noexcept {
	return m_availableMip;
}
void vulkan::Image::set_available_mip(uint32_t mip) noexcept {
	if (mip <= m_mipTail)
		m_availableMip = mip;
}
bool vulkan::Image::is_sparse() const noexcept {
	return m_info.flags & (VK_IMAGE_CREATE_SPARSE_BINDING_BIT | VK_IMAGE_CREATE_SPARSE_RESIDENCY_BIT);
}
uint32_t vulkan::Image::get_mip_tail() const noexcept {
	return m_mipTail;
}
uint32_t vulkan::Image::get_width(uint32_t mipLevel) const noexcept {
	return Math::max(1u, m_info.width >> mipLevel);
}
uint32_t vulkan::Image::get_height(uint32_t mipLevel) const noexcept {
	return Math::max(1u, m_info.height >> mipLevel);
}
uint32_t vulkan::Image::get_depth(uint32_t mipLevel) const noexcept {
	return Math::max(1u, m_info.depth >> mipLevel);
}
VkImageUsageFlags vulkan::Image::get_usage() const noexcept {
	return m_info.usage;
}
const vulkan::ImageInfo& vulkan::Image::get_info() const {
	return m_info;
}
VkFormat vulkan::Image::get_format() const {
	return m_info.format;
}
void vulkan::Image::set_optimal(bool optimal) noexcept {
	m_optimal = optimal;
}
bool vulkan::Image::is_optimal() const noexcept {
	return m_optimal;
}
void vulkan::Image::disown_image() noexcept {
	m_ownsImage = false;
}
void vulkan::Image::set_layout(VkImageLayout format) noexcept {
	m_info.layout = format;
}
void vulkan::Image::disown() noexcept {
	disown_image();
}
VkPipelineStageFlags vulkan::Image::get_stage_flags() const noexcept {
	return m_stageFlags;
}
VkAccessFlags vulkan::Image::get_access_flags() const noexcept {
	return m_accessFlags;
}
void vulkan::Image::set_stage_flags(VkPipelineStageFlags flags)  noexcept {
	m_stageFlags = flags;
}
void vulkan::Image::set_access_flags(VkAccessFlags flags) noexcept {
	m_accessFlags = flags;
}
void vulkan::Image::set_single_mip_tail(bool mipTail) noexcept {
	m_singleMipTail = mipTail;
}
bool vulkan::Image::is_being_resized() const noexcept {
	return m_isBeingResized;
}
void vulkan::Image::set_being_resized(bool resized) noexcept {
	m_isBeingResized = resized;
}