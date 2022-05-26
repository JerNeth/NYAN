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
	r_device.get_vma_allocator()->flush(m_allocation, offset,static_cast<uint32_t>(size == ~0u ? m_info.size : size));
}
void vulkan::Buffer::invalidate(uint32_t offset, uint32_t size)
{
	r_device.get_vma_allocator()->invalidate(m_allocation, offset, static_cast<uint32_t>(size == ~0u ? m_info.size : size));
}
VkDeviceAddress vulkan::Buffer::get_address() const
{
	assert(m_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	VkBufferDeviceAddressInfo info
	{
		.sType { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO },
		.pNext { nullptr },
		.buffer { m_vkHandle },
	};
	return vkGetBufferDeviceAddress(r_device, &info);
}
vulkan::Buffer::~Buffer()
{
	if(maped)
		r_device.get_vma_allocator()->unmap_memory(m_allocation);
	r_device.queue_buffer_deletion(m_vkHandle);
	r_device.queue_allocation_deletion(m_allocation);
}