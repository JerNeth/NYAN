#pragma once
#include <expected>
#include <filesystem>
#include <array>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"

namespace vulkan
{

	class PipelineCache : public VulkanObject<VkPipelineCache> {
		//Curtesy of https://zeux.io/2019/07/17/serializing-pipeline-cache/
		struct PipelineCachePrefixHeader {
			static constexpr uint32_t magicNumberValue = 0x68636163u;
			uint32_t magicNumber; // an arbitrary magic header to make sure this is actually our file
			uint32_t dataSize; // equal to *pDataSize returned by vkGetPipelineCacheData
			uint64_t dataHash; // a hash of pipeline cache data, including the header

			uint32_t vendorID; // equal to VkPhysicalDeviceProperties::vendorID
			uint32_t deviceID; // equal to VkPhysicalDeviceProperties::deviceID
			uint32_t driverVersion; // equal to VkPhysicalDeviceProperties::driverVersion
			uint32_t driverABI; // equal to sizeof(void*)

			std::array<uint8_t, VK_UUID_SIZE>  uuid; // equal to VkPhysicalDeviceProperties::pipelineCacheUUID

			friend bool operator==(const PipelineCachePrefixHeader& left, const PipelineCachePrefixHeader& right) noexcept = default;
		};
		PipelineCache(LogicalDevice& device, VkPipelineCache handle, std::filesystem::path path) noexcept;
	public:
		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&&) noexcept;
		PipelineCache& operator=(PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) noexcept;
		~PipelineCache();

		[[nodiscard]] static std::expected<PipelineCache, vulkan::Error> create(LogicalDevice& device, std::filesystem::path path) noexcept;
	private:
		std::filesystem::path m_path;
	};
	
}
