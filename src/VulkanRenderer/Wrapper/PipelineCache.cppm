module;

//#include <array>
//#include <expected>
//#include <span>

#include "volk.h"

export module NYANVulkan:PipelineCache;
import std;

import NYANData;

import :PhysicalDevice;
import :LogicalDeviceWrapper;
import :Error;
import :Object;

export namespace nyan::vulkan
{
	class LogicalDevice;

	class PipelineCache : public Object<VkPipelineCache>
	{
	private:
		//Curtesy of https://zeux.io/2019/07/17/serializing-pipeline-cache/
		struct PipelineCachePrefixHeader {
			static constexpr uint32_t magicNumberValue = 0x68636163u;
			uint32_t magicNumber; // an arbitrary magic header to make sure this is actually our file
			uint32_t dataSize; // equal to *pDataSize returned by vkGetPipelineCacheData
			HashValue dataHash; // a hash of pipeline cache data, including the header

			uint32_t vendorID; // equal to VkPhysicalDeviceProperties::vendorID
			uint32_t deviceID; // equal to VkPhysicalDeviceProperties::deviceID
			uint32_t driverVersion; // equal to VkPhysicalDeviceProperties::driverVersion
			uint32_t driverABI; // equal to sizeof(void*)

			std::array<uint8_t, VK_UUID_SIZE>  uuid; // equal to VkPhysicalDeviceProperties::pipelineCacheUUID

			friend bool operator==(const PipelineCachePrefixHeader& left, const PipelineCachePrefixHeader& right) noexcept = default;
		};
	public:
		//using ArrayType = util::data::DynArray;

		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&& other) noexcept;

		PipelineCache& operator=(PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) noexcept;

		~PipelineCache();

		DynamicArray<std::byte> get_data() const noexcept;

		[[nodiscard("must handle potential error")]] std::expected<void, Error> merge(const PipelineCache& other) noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> merge(std::span<PipelineCache> others) noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<PipelineCache, Error> create(const LogicalDevice& device, std::span<std::byte> data) noexcept;
	private:
		PipelineCache(const LogicalDeviceWrapper& device, VkPipelineCache handle, const  PhysicalDevice& physicalDevice) noexcept;

		const PhysicalDevice& r_physicalDevice;
	};
}
