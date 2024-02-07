module;

#include <cassert>
#include <utility>
#include <iostream>

#include "magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkanWrapper;

import :LogicalDevice;

using namespace nyan::vulkan::wrapper;

Buffer::Buffer(Buffer&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	m_data(other.m_data)
{
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	if (this != std::addressof(other)) 
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_data, other.m_data);
	}
	return *this;
}

Buffer::~Buffer() noexcept 
{
	if (m_handle != VK_NULL_HANDLE)
		return;
}

bool Buffer::shared() const noexcept
{
	return m_data.shared;
}

bool Buffer::mapable() const noexcept
{
	return m_data.mapable;
}

std::expected<Buffer, Error> Buffer::create(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Usage usage) noexcept
{

	//vma
	return Buffer{ device, deletionQueue , VK_NULL_HANDLE, usage};
}

Buffer::Buffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkBuffer handle, Usage usage) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	m_data{ .usage = usage }
{
	assert(!m_data.usage.test(UsageFlags::VideoDecodeSrc) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoDecodeDst) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoEncodeSrc) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoEncodeDst) && "Not supported");
	assert(m_handle != VK_NULL_HANDLE);

}

std::expected<StorageBuffer, Error> StorageBuffer::create(LogicalDevice& device, size_t size, const Options& options) noexcept
{
	//VmaAllocator allocator,
	//	const VkBufferCreateInfo* pBufferCreateInfo,
	//	const VmaAllocationCreateInfo* pAllocationCreateInfo,
	//	VkBuffer* pBuffer,
	//	VmaAllocation* pAllocation,
	//	VmaAllocationInfo* pAllocationInfo
	VkBufferCreateInfo bufferCreateInfo{
		.sType {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
		.pNext {nullptr},
		.size {size},
		.usage {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT},
		.sharingMode {VK_SHARING_MODE_EXCLUSIVE },
		.queueFamilyIndexCount {0},
		.pQueueFamilyIndices {nullptr},
	};
	if (!options.queueFamilies.empty()) {
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = options.queueFamilies.size();
		bufferCreateInfo.pQueueFamilyIndices = options.queueFamilies.data();
	}


	VmaAllocationCreateInfo allocationInfo
	{
		.flags {},
		.usage {VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
	};

	if (options.dedicatedAllocation)
		allocationInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	if (options.mapable)
		allocationInfo.flags |=
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		//VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT;


	VkBuffer buffer;
	VmaAllocation allocation;

	if (auto result = vmaCreateBuffer(device.get_allocator().get_handle(), &bufferCreateInfo, &allocationInfo, &buffer, &allocation, nullptr); result != VK_SUCCESS) {
		return std::unexpected{result};
	}

	VkMemoryPropertyFlags memPropFlags;
	vmaGetAllocationMemoryProperties(device.get_allocator().get_handle(), allocation, &memPropFlags);
	assert(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	return StorageBuffer{ device.get_device(), device.get_deletion_queue(), buffer };
}


StorageBuffer::StorageBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkBuffer handle) noexcept :
	Buffer(device, deletionQueue, handle, { Buffer::UsageFlags::StorageBuffer })
{

}