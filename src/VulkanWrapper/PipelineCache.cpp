#include "VulkanWrapper/PipelineCache.hpp"

#include <fstream>

#include "VulkanWrapper/LogicalDevice.h"

vulkan::PipelineCache::PipelineCache(LogicalDevice& device, VkPipelineCache handle,
	std::filesystem::path path) noexcept :
	VulkanObject(device, handle),
	m_path(std::move(path))
{
}

vulkan::PipelineCache::PipelineCache(PipelineCache&& other) noexcept :
	VulkanObject(other.r_device, other.m_handle),
	m_path(other.m_path)
{
	other.m_handle = VK_NULL_HANDLE;
}

vulkan::PipelineCache& vulkan::PipelineCache::operator=(PipelineCache&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	if(this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_path, other.m_path);
	}
	return *this;
}
vulkan::PipelineCache::~PipelineCache()
{
	if (m_handle == VK_NULL_HANDLE)
		return;
	size_t dataSize;
	VkResult result;
	std::vector<std::byte> data;
	do {
		r_device.get_device().vkGetPipelineCacheData(m_handle, &dataSize, nullptr);
		data.resize(dataSize + sizeof(PipelineCachePrefixHeader));
		result = r_device.get_device().vkGetPipelineCacheData(m_handle, &dataSize, data.data() + sizeof(PipelineCachePrefixHeader));
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			r_device.get_device().vkDestroyPipelineCache(m_handle, r_device.get_allocator());
			return;
		}
		else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			r_device.get_device().vkDestroyPipelineCache(m_handle, r_device.get_allocator());
			return;
		}
	} while (result != VK_SUCCESS);

	PipelineCachePrefixHeader* currentHeader = reinterpret_cast<PipelineCachePrefixHeader*>(data.data());
	assert(dataSize <= std::numeric_limits<uint32_t>::max());
	*currentHeader = PipelineCachePrefixHeader{
		.magicNumber { PipelineCachePrefixHeader::magicNumberValue},
		.dataSize {static_cast<uint32_t>(dataSize)},
		.dataHash {},
		.vendorID {r_device.get_physical_device_properties().vendorID},
		.deviceID {r_device.get_physical_device_properties().deviceID},
		.driverVersion {r_device.get_physical_device_properties().driverVersion},
		.driverABI {sizeof(void*)},
		.uuid{std::to_array(r_device.get_physical_device_properties().pipelineCacheUUID)}
	};
	currentHeader->dataHash = Utility::DataHash{}(data.data() + offsetof(PipelineCachePrefixHeader, vendorID), data.size() - offsetof(PipelineCachePrefixHeader, vendorID));
	std::filesystem::path tmpPath = m_path.stem().string() + ".tmp";
	std::ofstream out(tmpPath, std::ios::binary);
	if (out.is_open()) {
		out.write(reinterpret_cast<char*>(data.data()), data.size());
		out.close();
	}
	std::filesystem::rename(tmpPath, m_path);
	r_device.get_device().vkDestroyPipelineCache(m_handle, r_device.get_allocator());
}

std::expected<vulkan::PipelineCache, vulkan::Error> vulkan::PipelineCache::create(LogicalDevice& device,
	std::filesystem::path path) noexcept
{
	std::vector<std::byte> data;
	std::ifstream in(path, std::ios::binary);
	if (in.is_open()) {
		in.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = in.gcount();
		if (length > 0) {
			in.clear();   //  Since ignore will have set eof.
			in.seekg(0, std::ios_base::beg);
			data.resize(length);
			in.read(reinterpret_cast<char*>(data.data()), length);
			in.close();
		}
	}

	VkPipelineCacheCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.initialDataSize = 0,
		.pInitialData = nullptr,
	};
	if (data.size() > sizeof(PipelineCachePrefixHeader)) {
		PipelineCachePrefixHeader header = *reinterpret_cast<PipelineCachePrefixHeader*>(data.data());
		PipelineCachePrefixHeader currentHeader{
			.magicNumber { PipelineCachePrefixHeader::magicNumberValue},
			.dataSize {header.dataSize},
			.dataHash {Utility::DataHash{}(data.data() + offsetof(PipelineCachePrefixHeader, vendorID), data.size() - offsetof(PipelineCachePrefixHeader, vendorID))},
			.vendorID {device.get_physical_device_properties().vendorID},
			.deviceID {device.get_physical_device_properties().deviceID},
			.driverVersion {device.get_physical_device_properties().driverVersion},
			.driverABI {sizeof(void*)},
			.uuid{std::to_array(device.get_physical_device_properties().pipelineCacheUUID)}
		};
		if (currentHeader == header && header.dataSize > 0) {
			createInfo.initialDataSize = header.dataSize;
			createInfo.pInitialData = data.data() + sizeof(PipelineCachePrefixHeader);
		}
		else {
			Utility::Logger::warning_message("Existing pipeline cache invalid");
		}
	}
	VkPipelineCache handle{VK_NULL_HANDLE};
	if (auto result = device.get_device().vkCreatePipelineCache(&createInfo, device.get_allocator(), &handle); result != VK_SUCCESS) {
		Utility::Logger::warning_message("Couldn't create pipeline cache");
		return std::unexpected{ vulkan::Error{result} };
	}

	return vulkan::PipelineCache(device, handle, path);
}