module;

//#include <cassert>
//#include <expected>
//#include <utility>

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkan;
import std;

import :Instance;
import :PhysicalDevice;

using namespace nyan::vulkan;

Allocator::Allocator(Allocator&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE))
{
}

Allocator& Allocator::operator=(Allocator&& other) noexcept
{
	::assert(ptr_device == other.ptr_device);
	if(this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

Allocator::~Allocator()
{
	if (m_handle != VK_NULL_HANDLE)
		vmaDestroyAllocator(m_handle);
}

std::expected<void*, Error> Allocator::map_memory(const VmaAllocation allocation) const noexcept

{
	void* data{ nullptr };

	if (const auto result = vmaMapMemory(m_handle, allocation, &data); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ Error{result} };
	return data;
}

void Allocator::unmap_memory(const VmaAllocation allocation) const noexcept
{
	vmaUnmapMemory(m_handle, allocation);
}

std::expected<void, Error> Allocator::flush(const VmaAllocation allocation, VkDeviceSize offset,
	VkDeviceSize size) const noexcept
{
	if (const auto result = vmaFlushAllocation(m_handle, allocation, offset, size); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ Error{result} };
	return {};
}

std::expected<void, Error> Allocator::invalidate(const VmaAllocation allocation, VkDeviceSize offset,
	VkDeviceSize size) const noexcept
{
	if (const auto result = vmaFlushAllocation(m_handle, allocation, offset, size); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ Error{result} };
	return {};
}

void Allocator::free_allocation(VmaAllocation allocation) const noexcept

{
	vmaFreeMemory(m_handle, allocation);
}

//std::expected<Allocator, Error> Allocator::create(const Instance& instance, const LogicalDeviceWrapper& logicalDevice, const PhysicalDevice& physicalDevice,
//                                                  VmaAllocatorCreateFlags createFlags) noexcept
//{
//	::VmaAllocator allocator;
//	if (const auto result = logicalDevice.vmaCreateAllocator(createFlags, physicalDevice.get_handle(),
//	instance.get_handle(), physicalDevice.get_properties().apiVersion, &allocator); result != VK_SUCCESS) {
//		return std::unexpected{ Error{result} };
//	}
//	return Allocator{logicalDevice, allocator };
//}

Allocator::Allocator(const LogicalDeviceWrapper& device, VmaAllocator allocator) noexcept :
	Object(device, allocator)
{
}
