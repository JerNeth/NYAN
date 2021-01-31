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
