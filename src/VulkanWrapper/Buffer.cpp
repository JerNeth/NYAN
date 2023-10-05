#include "VulkanWrapper/Buffer.h"

#include "VulkanWrapper/LogicalDevice.h"


vulkan::Buffer::Buffer(LogicalDevice& device, VkBuffer buffer, VmaAllocation allocation, const BufferInfo& info):
	VulkanObject(device, buffer), 
	m_allocation(allocation),
	m_info(info)
{
	
}

VkBufferUsageFlags vulkan::Buffer::get_usage() const noexcept {
	return m_info.usage;
}

VmaMemoryUsage vulkan::Buffer::get_memory_usage() const noexcept {
	return m_info.memoryUsage;
}

VkDeviceSize vulkan::Buffer::get_size() const noexcept {
	return m_info.size;
}

const vulkan::BufferInfo& vulkan::Buffer::get_info() const noexcept {
	return m_info;
}

void vulkan::Buffer::swap_contents(Buffer& other) noexcept {
	//auto tmp = m_vkHandle;
	//auto tmpAll = m_allocation;
	//auto tmpInf = m_info;
	//m_vkHandle = other.m_vkHandle;
	//m_allocation = other.m_allocation;
	//m_info = other.m_info;
	//other.m_vkHandle = tmp;
	//other.m_allocation = tmpAll;
	//other.m_info = tmpInf;
	std::swap(m_handle, other.m_handle);
	std::swap(m_allocation, other.m_allocation); 
	std::swap(m_info, other.m_info);
}

void* vulkan::Buffer::map_data() noexcept
{
	if(!maped)
		maped = *r_device.get_vma_allocator().map_memory(m_allocation);
	return maped;
}
void vulkan::Buffer::unmap_data() noexcept
{
	if (maped)
		r_device.get_vma_allocator().unmap_memory(m_allocation);
	maped = nullptr;
}
void vulkan::Buffer::flush(uint32_t offset, uint32_t size)
{
	r_device.get_vma_allocator().flush(m_allocation, offset,static_cast<uint32_t>(size == ~0u ? m_info.size : size));
}
void vulkan::Buffer::invalidate(uint32_t offset, uint32_t size)
{
	r_device.get_vma_allocator().invalidate(m_allocation, offset, static_cast<uint32_t>(size == ~0u ? m_info.size : size));
}
VkDeviceAddress vulkan::Buffer::get_address() const
{
	assert(m_handle != VK_NULL_HANDLE);
	assert(m_info.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
	VkBufferDeviceAddressInfo info
	{
		.sType { VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO },
		.pNext { nullptr },
		.buffer { m_handle },
	};
	return r_device.get_device().vkGetBufferDeviceAddress( &info);
}

vulkan::Buffer::~Buffer()
{
	if(maped)
		r_device.get_vma_allocator().unmap_memory(m_allocation);
	if(m_handle)
		r_device.get_deletion_queue().queue_buffer_deletion(m_handle);
	if(m_allocation)
		r_device.get_deletion_queue().queue_allocation_deletion(m_allocation);
}