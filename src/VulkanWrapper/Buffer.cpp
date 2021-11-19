#include "Buffer.h"
#include "LogicalDevice.h"

void vulkan::Buffer::resize(VkDeviceSize newSize, bool copyData) {
	//if (newSize == get_size())
	//	return;
	r_device.resize_buffer(*this, newSize, copyData);
}

//vulkan::MappedMemoryHandle<uint8_t> vulkan::Buffer::map_data() const noexcept
//{
//	return MappedMemoryHandle<uint8_t>(r_device.get_vma_allocator(), m_allocation);
//}
void* vulkan::Buffer::map_data() noexcept
{
	if(!maped)
		r_device.get_vma_allocator()->map_memory(m_allocation, reinterpret_cast<void**>(&maped));
	return maped;
}
void vulkan::Buffer::unmap_data() noexcept
{
	if (maped)
		r_device.get_vma_allocator()->unmap_memory(m_allocation);
	maped = nullptr;
}
void vulkan::Buffer::flush(uint32_t offset, uint32_t size)
{
	auto updateSize = size == ~0u ? m_info.size : size;
	r_device.get_vma_allocator()->flush(m_allocation, offset, updateSize);
}
vulkan::Buffer::~Buffer()
{
	if(maped)
		r_device.get_vma_allocator()->unmap_memory(m_allocation);
	r_device.queue_buffer_deletion(m_vkHandle);
	r_device.queue_allocation_deletion(m_allocation);
}