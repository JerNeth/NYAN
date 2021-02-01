#include "Allocator.h"

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
Vulkan::ImageView* Vulkan::AttachmentAllocator::request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index, uint32_t samples, uint32_t layers)
{
	Utility::Hasher hasher;
	hasher(width);
	hasher(height);
	hasher(format);
	hasher(index);
	hasher(samples);
	auto hash = hasher(layers);
	if (auto res = m_attachmentIds.find(hash); res != m_attachmentIds.end())
		return m_imageViewStorage.get(res->second);
	ImageInfo info = ImageInfo::render_target(width, height, format);
	m_imageStorage.emplace(r_device,info, VMA_MEMORY_USAGE_GPU_ONLY);
	return nullptr;
}