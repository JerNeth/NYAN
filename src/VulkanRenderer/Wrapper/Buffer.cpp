module;

#include <cassert>
#include <utility>

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkanWrapper;

using namespace nyan::vulkan::wrapper;

Buffer::Buffer(Buffer&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE))
{
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	if (this != std::addressof(other)) 
	{
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

std::expected<Buffer, Error> Buffer::create(const LogicalDeviceWrapper& device) noexcept
{

	vma
	return Buffer{ device , VK_NULL_HANDLE};
}


Buffer::Buffer(const LogicalDeviceWrapper& device, VkBuffer handle) noexcept :
	Object(device, handle)
{
	assert(m_handle != VK_NULL_HANDLE);
}
