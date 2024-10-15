module;

//#include <array>
//#include <cassert>
//#include <expected>
//#include <span>
//#include <limits>

#include "volk.h"

module NYANVulkan;
import std;

import NYANData;
import NYANLog;

using namespace nyan::vulkan;


PipelineCache::PipelineCache(PipelineCache&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_physicalDevice(other.r_physicalDevice)
{
}

PipelineCache& PipelineCache::operator=(PipelineCache&& other) noexcept
{
	::assert(ptr_device == other.ptr_device);
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

PipelineCache::~PipelineCache()
{
	if (m_handle)
		ptr_device->vkDestroyPipelineCache(m_handle);
}

nyan::DynamicArray<std::byte> PipelineCache::get_data() const noexcept
{

	size_t dataSize;
	nyan::DynamicArray<std::byte> retVal;
	if (const auto result = ptr_device->vkGetPipelineCacheData(m_handle, &dataSize, nullptr); result != VK_SUCCESS)
		return retVal;
	if (!retVal.resize(sizeof(PipelineCachePrefixHeader) + dataSize))
		return retVal;
	if (const auto result = ptr_device->vkGetPipelineCacheData(m_handle, &dataSize, retVal.data() + sizeof(PipelineCachePrefixHeader)); result != VK_SUCCESS)
		return retVal;

	::assert(dataSize <= std::numeric_limits<uint32_t>::max());
	if (dataSize > std::numeric_limits<uint32_t>::max())
		return retVal;

	const auto& properties = r_physicalDevice.get_properties();
	PipelineCachePrefixHeader* currentHeader = new(retVal.data()) PipelineCachePrefixHeader {
		.magicNumber { PipelineCachePrefixHeader::magicNumberValue},
		.dataSize {static_cast<uint32_t>(dataSize)},
		.dataHash {},
		.vendorID {properties.vendorID},
		.deviceID {properties.deviceID},
		.driverVersion {properties.driverVersion},
		.driverABI {static_cast<uint32_t>(sizeof(void*))},
		.uuid{std::to_array(properties.pipelineCacheUUID)}
	};
	
	currentHeader->dataHash = nyan::hash(std::span{ retVal.data() + offsetof(PipelineCachePrefixHeader, vendorID), retVal.size() - offsetof(PipelineCachePrefixHeader, vendorID) });

	return retVal;
}

std::expected<void, Error> PipelineCache::merge(const PipelineCache& other) noexcept
{
	if (const auto result = ptr_device->vkMergePipelineCaches(m_handle, 1, &other.get_handle()); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{result};

	return { };
}

std::expected<void, Error> PipelineCache::merge(std::span<PipelineCache> others) noexcept
{
	DynamicArray<VkPipelineCache> caches;
	if(caches.reserve(others.size())) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };
	for (const auto& cache : others)
		if(caches.push_back(cache.get_handle())) [[unlikely]]
			return std::unexpected{ VK_ERROR_UNKNOWN };

	if (const auto result = ptr_device->vkMergePipelineCaches(m_handle, caches.size(), caches.data()); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return { };
}

std::expected<PipelineCache, Error> PipelineCache::create(const LogicalDevice& device, std::span<std::byte> data) noexcept
{
	VkPipelineCacheCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = nullptr,
	};
	if (device.get_enabled_extensions().pipelineCreationCacheControl)
		createInfo.flags = VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;

	if (data.size() > sizeof(PipelineCachePrefixHeader)) {
		const auto& properties = device.get_physical_device().get_properties();
		PipelineCachePrefixHeader header = *reinterpret_cast<PipelineCachePrefixHeader*>(data.data());
		PipelineCachePrefixHeader currentHeader{
			.magicNumber { PipelineCachePrefixHeader::magicNumberValue},
			.dataSize {header.dataSize},
			.dataHash {nyan::hash(std::span{ data.data() + offsetof(PipelineCachePrefixHeader, vendorID), data.size() - offsetof(PipelineCachePrefixHeader, vendorID) })},
			.vendorID {properties.vendorID},
			.deviceID {properties.deviceID},
			.driverVersion {properties.driverVersion},
			.driverABI {static_cast<uint32_t>(sizeof(void*))},
			.uuid{std::to_array(properties.pipelineCacheUUID)}
		};
		if (currentHeader == header && header.dataSize > 0 && (header.dataSize <= (data.size() - sizeof(PipelineCachePrefixHeader)))) {
			createInfo.initialDataSize = header.dataSize;
			createInfo.pInitialData = data.data() + sizeof(PipelineCachePrefixHeader);
		}
		else {
			log::warning().message("Existing pipeline cache invalid");
		}
	}
	VkPipelineCache handle{ VK_NULL_HANDLE };
	const auto& deviceWrapper = device.get_device();
	if (auto result = deviceWrapper.vkCreatePipelineCache(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]] {
		log::warning().message("Couldn't create pipeline cache");
		return std::unexpected{ result};
	}
	
	return PipelineCache{ deviceWrapper, handle, device.get_physical_device() };
}

PipelineCache::PipelineCache(const LogicalDeviceWrapper& device, const VkPipelineCache handle, const PhysicalDevice& physicalDevice) noexcept :
	Object(device, handle),
	r_physicalDevice(physicalDevice)
{
}
