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
	r_allocator(other.r_allocator),
	m_data(other.m_data)
{
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	assert(std::addressof(r_allocator) == std::addressof(other.r_allocator));
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
		r_deletionQueue.queue_buffer_deletion(m_handle);
	if (m_data.allocation != VK_NULL_HANDLE)
		r_deletionQueue.queue_allocation_deletion(m_data.allocation);
}

bool Buffer::shared() const noexcept
{
	return m_data.queueFamilies.size() > 1;
}

void* nyan::vulkan::wrapper::Buffer::mapped_data() const noexcept
{
	return m_data.ptr;
}

std::expected<void, Error> nyan::vulkan::wrapper::Buffer::flush() const noexcept
{
	if (auto res = r_allocator.flush(m_data.allocation, 0, m_data.size); !res)
		return std::unexpected{ res.error() };

	return {};
}

std::expected<void, Error> nyan::vulkan::wrapper::Buffer::invalidate() const noexcept
{

	if (auto res = r_allocator.invalidate(m_data.allocation, 0, m_data.size); !res)
		return std::unexpected{ res.error() };

	return {};
}

//std::expected<Buffer, Error> Buffer::create(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Usage usage) noexcept
//{
//
//	//vma
//	return Buffer{ device, deletionQueue , VK_NULL_HANDLE, usage};
//}

[[nodiscard]] VkBufferUsageFlagBits  convert_buffer_usage_flags(Buffer::UsageFlags usageFlags)
{
	switch (usageFlags) {
		using enum Buffer::UsageFlags;
		case TransferSrc: 
			return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		case TransferDst: 
			return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case UniformTexelBuffer: 
			return VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT;
		case StorageTexelBuffer: 
			return VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT;
		case UniformBuffer: 
			return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case StorageBuffer: 
			return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case IndexBuffer: 
			return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case VertexBuffer: 
			return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case IndirectBuffer: 
			return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		case ShaderDeviceAddress: 
			return VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
		case VideoDecodeSrc: 
			return VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR;
		case VideoDecodeDst: 
			return VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR;
		case AccelerationStructureBuildInputReadOnly: 
			return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
		case AccelerationStructureStorageBit: 
			return VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
		case ShaderBindingTable: 
			return VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
		case VideoEncodeSrc: 
			return VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR;
		case VideoEncodeDst: 
			return VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR;
		case SamplerDescriptorBuffer: 
			return VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT;
		case ResourceDescriptorBuffer: 
			return VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT;
		case PushDescriptorDescriptorBuffer: 
			return VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT;
		case MicromapBuildInput: 
			return VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT;
		case MicromapStorage: 
			return VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT;
		default:
			std::unreachable();
	}
}

[[nodiscard]] VkBufferUsageFlags convert_buffer_usage(Buffer::Usage usage)
{
	VkBufferUsageFlags usageFlags{};
	usage.for_each([&usageFlags](Buffer::UsageFlags usage) {usageFlags |= convert_buffer_usage_flags(usage); });
	return usageFlags;
}

Buffer::Buffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	r_allocator(allocator),
	m_data{ std::move(data)}
{
	assert(!m_data.usage.test(UsageFlags::VideoDecodeSrc) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoDecodeDst) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoEncodeSrc) && "Not supported");
	assert(!m_data.usage.test(UsageFlags::VideoEncodeDst) && "Not supported");
	assert(m_handle != VK_NULL_HANDLE);

}

std::expected<StorageBuffer, Error> StorageBuffer::create(LogicalDevice& device, const Options& options) noexcept
{
	static constexpr bool supportLargerBuffers = true; 
	//Can bind the same buffer multiple times, but targeted renderer not designed for that purpose
	
	if (const auto& physicalDevice = device.get_physical_device(); 
		!supportLargerBuffers ||
		options.size > physicalDevice.get_properties().limits.maxStorageBufferRange) {
		//TODO proper error
		assert(false && "Can't bind larger storage buffers in a single bind"); 
		return std::unexpected{ VK_ERROR_OUT_OF_DEVICE_MEMORY };
	}

	if (const auto& physicalDevice = device.get_physical_device(); 
		physicalDevice.get_vulkan13_features().maintenance4 &&
		options.size > physicalDevice.get_vulkan13_properties().maxBufferSize) {
		//TODO proper error
		assert(false && "Desired buffer exceeds maximum supported size");
		return std::unexpected{ VK_ERROR_OUT_OF_DEVICE_MEMORY };
	}

	Buffer::Usage usage{ Buffer::UsageFlags::StorageBuffer, Buffer::UsageFlags::TransferDst };

	std::array<uint32_t, 32> queueFamilies;
	VkBufferCreateInfo bufferCreateInfo{
		.sType {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO },
		.pNext {nullptr},
		.flags {0},
		.size {options.size},
		.usage {convert_buffer_usage(usage)},
		.sharingMode {VK_SHARING_MODE_EXCLUSIVE },
		.queueFamilyIndexCount {0},
		.pQueueFamilyIndices {queueFamilies.data()},
	};

	if (options.queueFamilies.size() > 1) {
		options.queueFamilies.fill(std::span{ queueFamilies.data(), options.queueFamilies.size()});
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		bufferCreateInfo.queueFamilyIndexCount = options.queueFamilies.size();
	}


	VmaAllocationCreateInfo allocationCreateInfo
	{
		.flags {},
		.usage {VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE},
	};

	if (options.dedicatedAllocation)
		allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	if (options.mapable)
		allocationCreateInfo.flags |=
		VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
		//VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT |
		VMA_ALLOCATION_CREATE_MAPPED_BIT;

	auto allocatorHandle = device.get_allocator().get_handle();

	VkBuffer buffer;
	VmaAllocation allocation;
	VmaAllocationInfo allocationInfo;
	if (auto result = vmaCreateBuffer(allocatorHandle, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, &allocationInfo); result != VK_SUCCESS)
		return std::unexpected{result};

	VkMemoryPropertyFlags memPropFlags;
	vmaGetAllocationMemoryProperties(allocatorHandle, allocation, &memPropFlags);


	assert(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	if (!(memPropFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
		vmaDestroyBuffer(allocatorHandle, buffer, allocation);
		return std::unexpected{ VK_ERROR_OUT_OF_DEVICE_MEMORY };
	}

	return StorageBuffer{ device.get_device(), device.get_deletion_queue(), device.get_allocator(), buffer,
		Buffer::Data {
			.usage{ usage },
			.location{ Buffer::Location::Device},
			.queueFamilies{options.queueFamilies},
			.size{options.size},
			.allocation{allocation},
			.ptr {allocationInfo.pMappedData}
		}
	};

}


StorageBuffer::StorageBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept :
	Buffer(device, deletionQueue, allocator, handle, data)
{
	
	//m_data.usage.to_ullong(magic_enum::containers::detail::raw_access_t{});
}