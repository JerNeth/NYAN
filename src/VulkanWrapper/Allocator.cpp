#include "Allocator.h"
#include "LogicalDevice.h"

Vulkan::Allocator::Allocator(VmaAllocator handle) :
	m_VmaHandle(handle)
{
}

Vulkan::Allocator::Allocator(Allocator&& other) noexcept :
	m_VmaHandle(other.m_VmaHandle)
{
	other.m_VmaHandle = VK_NULL_HANDLE;
}

Vulkan::Allocator::~Allocator() noexcept
{
	if(m_VmaHandle != VK_NULL_HANDLE)
		vmaDestroyAllocator(m_VmaHandle);
}

void Vulkan::Allocator::map_memory(VmaAllocation allocation, void** data)
{
	vmaMapMemory(m_VmaHandle, allocation, data);
}

void Vulkan::Allocator::unmap_memory(VmaAllocation allocation)
{
	vmaUnmapMemory(m_VmaHandle, allocation);
}

void Vulkan::Allocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
{
	vmaDestroyBuffer(m_VmaHandle, buffer, allocation);
}

void Vulkan::Allocator::flush(VmaAllocation allocation, uint32_t offset, uint32_t size)
{

	vmaFlushAllocation(m_VmaHandle, allocation, offset, size);
}
Vulkan::AttachmentAllocator::AttachmentAllocator(LogicalDevice& parent) :
	r_device(parent)
{
}
Vulkan::ImageView* Vulkan::AttachmentAllocator::request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index, VkSampleCountFlagBits samples)
{
	Utility::Hasher hasher;
	hasher(width);
	hasher(height);
	hasher(format);
	hasher(index);
	auto hash = hasher(samples);
	if (auto res = m_attachmentIds.find(hash); res != m_attachmentIds.end())
		return res->second->get_view();
	ImageInfo info = ImageInfo::render_target(width, height, format);
	info.usage |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	return m_attachmentIds.emplace(hash, r_device.create_image(info)).first->second->get_view();
}