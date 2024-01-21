#include "VulkanWrapper/RayTracePipeline.h"

#include "Utility/Exceptions.h"

#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/Pipeline.hpp"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/LogicalDevice.h"


vulkan::RTPipeline::RTPipeline(vulkan::LogicalDevice& device, const vulkan::RaytracingPipelineConfig& config) :
	r_device(device),
	m_config(),
	m_rtPipeline(create_pipeline(config)),
	m_sbt(create_sbt(config))
{
}

vulkan::RTPipeline::operator vulkan::PipelineId() const
{
	return m_rtPipeline;
}

const VkStridedDeviceAddressRegionKHR* vulkan::RTPipeline::rgen_region() const
{
	return &m_rgenRegion;
}
const VkStridedDeviceAddressRegionKHR* vulkan::RTPipeline::miss_region() const
{
	return &m_missRegion;
}
const VkStridedDeviceAddressRegionKHR* vulkan::RTPipeline::hit_region() const
{
	return &m_hitRegion;
}
const VkStridedDeviceAddressRegionKHR* vulkan::RTPipeline::callable_region() const
{
	return &m_callableRegion;
}

vulkan::PipelineId vulkan::RTPipeline::create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig)
{
	return r_device.get_pipeline_storage().add_pipeline(rayConfig);
}

vulkan::BufferHandle vulkan::RTPipeline::create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(m_rtPipeline);

	const auto& rtProperties = r_device.get_physical_device().get_ray_tracing_pipeline_properties();
	auto handleSize{ rtProperties.shaderGroupHandleSize };
	auto groupCount{ rayConfig.rgenGroups.size() + rayConfig.hitGroups.size() + rayConfig.missGroups.size() + rayConfig.callableGroups.size() };
	std::vector<std::byte> handleData(handleSize * groupCount);
	if (auto result = r_device.get_device().vkGetRayTracingShaderGroupHandlesKHR( *pipeline, 0, static_cast<uint32_t>(groupCount), handleData.size(), handleData.data()); result != VK_SUCCESS) {
		assert(false);
		throw Utility::VulkanException(result);
	}
	auto handleStride = Utility::align_up(handleSize, rtProperties.shaderGroupHandleAlignment);
	uint32_t rgenCount{ static_cast<uint32_t>(rayConfig.rgenGroups.size()) };
	uint32_t hitCount{ static_cast<uint32_t>(rayConfig.hitGroups.size()) };
	uint32_t missCount{ static_cast<uint32_t>(rayConfig.missGroups.size()) };
	uint32_t callableCount{ static_cast<uint32_t>(rayConfig.callableGroups.size()) };

	assert(rgenCount == 1); //only one rgen per sbt, possibly per pipeline but idk
	m_rgenRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = Utility::align_up(handleStride, rtProperties.shaderGroupBaseAlignment),
		.size = Utility::align_up(rgenCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_hitRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(hitCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_missRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(missCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_callableRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(callableCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};

	assert(m_rgenRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_hitRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_missRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_callableRegion.stride <= rtProperties.maxShaderGroupStride);
	auto bufferSize = m_rgenRegion.size + m_hitRegion.size + m_missRegion.size + m_callableRegion.size;

	std::vector<std::byte> stridedHandles(bufferSize);
	std::vector<vulkan::InputData> inputData{
		vulkan::InputData{
			.ptr {stridedHandles.data()},
			.size {bufferSize},
		}
	};
	size_t offset{ 0 };
	size_t handleCount{ 0 };
	for (size_t i{ 0 }; i < rgenCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_rgenRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_rgenRegion.size;
	for (size_t i{ 0 }; i < hitCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_hitRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_hitRegion.size;
	for (size_t i{ 0 }; i < missCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_missRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_missRegion.size;
	for (size_t i{ 0 }; i < callableCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_callableRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_callableRegion.size;


	auto sbt = r_device.create_buffer(vulkan::BufferInfo{
		.size = bufferSize,
		.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
		}, inputData);

	auto buffAddr = sbt->get_address();

	if (rgenCount)
		m_rgenRegion.deviceAddress = buffAddr;
	if (hitCount)
		m_hitRegion.deviceAddress = buffAddr + m_rgenRegion.size;
	if (missCount)
		m_missRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size;
	if (callableCount)
		m_callableRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size + m_missRegion.size;

	return sbt;
}
