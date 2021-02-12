#include "Allocator.h"
#include "LogicalDevice.h"

vulkan::Allocator::Allocator(VmaAllocator handle) :
	m_VmaHandle(handle)
{
}

vulkan::Allocator::Allocator(Allocator&& other) noexcept :
	m_VmaHandle(other.m_VmaHandle)
{
	other.m_VmaHandle = VK_NULL_HANDLE;
}

vulkan::Allocator::~Allocator() noexcept
{
	if(m_VmaHandle != VK_NULL_HANDLE)
		vmaDestroyAllocator(m_VmaHandle);
}

void vulkan::Allocator::map_memory(VmaAllocation allocation, void** data)
{
	vmaMapMemory(m_VmaHandle, allocation, data);
}

void vulkan::Allocator::unmap_memory(VmaAllocation allocation)
{
	vmaUnmapMemory(m_VmaHandle, allocation);
}

void vulkan::Allocator::destroy_buffer(VkBuffer buffer, VmaAllocation allocation)
{
	vmaDestroyBuffer(m_VmaHandle, buffer, allocation);
}

void vulkan::Allocator::flush(VmaAllocation allocation, uint32_t offset, uint32_t size)
{

	vmaFlushAllocation(m_VmaHandle, allocation, offset, size);
}
vulkan::AttachmentAllocator::AttachmentAllocator(LogicalDevice& parent) :
	r_device(parent)
{
}
vulkan::ImageView* vulkan::AttachmentAllocator::request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index, VkSampleCountFlagBits samples)
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

vulkan::Allocation::Allocation(LogicalDevice& device, VmaAllocation handle)
	:r_device(device),
	m_VmaHandle(handle)
{

}

vulkan::Allocation::~Allocation()
{
	if (m_VmaHandle != VK_NULL_HANDLE)
		r_device.queue_allocation_deletion(m_VmaHandle);
}
