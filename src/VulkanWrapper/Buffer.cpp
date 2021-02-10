#include "..\..\include\VulkanWrapper\Buffer.h"
#include "Buffer.h"
#include "LogicalDevice.h"

void Vulkan::Buffer::resize(VkDeviceSize newSize, bool copyData) {
	//if (newSize == get_size())
	//	return;
	r_device.resize_buffer(*this, newSize, copyData);
}

Vulkan::MappedMemoryHandle<uint8_t> Vulkan::Buffer::map_data() const noexcept
{
	return MappedMemoryHandle<uint8_t>(r_device.get_vma_allocator(), m_allocation);
}

Vulkan::Buffer::~Buffer()
{
	r_device.queue_buffer_deletion( m_vkHandle, m_allocation );
}