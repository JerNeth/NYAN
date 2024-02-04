module;

#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;

using namespace nyan::vulkan::wrapper;

GraphicsPipeline::GraphicsPipeline(GraphicsPipeline&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE))
{
	assert(m_handle != VK_NULL_HANDLE);
}

GraphicsPipeline& GraphicsPipeline::operator=(GraphicsPipeline&& other) noexcept
{
	if (this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

GraphicsPipeline::~GraphicsPipeline() noexcept
{
}

std::expected<GraphicsPipeline, Error> GraphicsPipeline::create(const LogicalDeviceWrapper& device, PipelineCache& pipelineCache) noexcept
{
	return GraphicsPipeline{ device , VK_NULL_HANDLE};
}

GraphicsPipeline::GraphicsPipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept :
	Object(device, handle)
{
	assert(m_handle != VK_NULL_HANDLE);
}

ComputePipeline::ComputePipeline(ComputePipeline&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE))
{
}

ComputePipeline& ComputePipeline::operator=(ComputePipeline&& other) noexcept
{
	if (this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

ComputePipeline::~ComputePipeline() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_device.vkDestroyPipeline(m_handle);
}

std::expected<ComputePipeline, Error> ComputePipeline::create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, const ShaderInstance& computeShader, PipelineCache& pipelineCache) noexcept
{
	VkPipelineLayout pipelineLayout { layout.get_handle()};
	assert(pipelineLayout != VK_NULL_HANDLE);
	//assert(config.shaderInstance != invalidShaderId);
	VkComputePipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // |VK_PIPELINE_CREATE_DISPATCH_BASE_BIT 
		.stage = computeShader.get_shader_stage_create_info(), //parent.get_shader_storage().get_instance(config.shaderInstance)->get_stage_info(),
		.layout = pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	//Validate VUID-VkComputePipelineCreateInfo-flags-xxxxx
	assert(!(createInfo.flags & (VK_PIPELINE_CREATE_LIBRARY_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV |
		VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV)));
	assert(createInfo.stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

	VkPipeline handle{ VK_NULL_HANDLE };
	if (auto result = device.vkCreateComputePipelines(pipelineCache.get_handle(), 1, &createInfo, &handle);
		result != VK_SUCCESS && result != VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		return std::unexpected{result};
	}
	return ComputePipeline{device, handle};
}

ComputePipeline::ComputePipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept :
	Object(device, handle)
{
	assert(m_handle != VK_NULL_HANDLE);
}

RayTracingPipeline::RayTracingPipeline(RayTracingPipeline&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE))
{
}

RayTracingPipeline& RayTracingPipeline::operator=(RayTracingPipeline&& other) noexcept
{
	if(this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

RayTracingPipeline::~RayTracingPipeline() noexcept
{
}

std::expected<RayTracingPipeline, Error> RayTracingPipeline::create(const LogicalDeviceWrapper& device, PipelineCache& pipelineCache) noexcept
{
	return RayTracingPipeline{device, VK_NULL_HANDLE};
}

RayTracingPipeline::RayTracingPipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept :
	Object(device, handle)
{
}
