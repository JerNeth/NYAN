// ReSharper disable CppMemberFunctionMayBeConst
// Functions are technically const but semantically they modify something
module;

#include <bit>
#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;
import :DescriptorSet;
import :Pipeline;

using namespace nyan::vulkan::wrapper;

void PipelineBind::push_descriptor_set(uint32_t firstSet, const StorageBuffer& buffer, VkDeviceSize offset, VkDeviceSize range) const noexcept
{
	assert(false && "TODO");
	VkDescriptorBufferInfo bufferInfo{
		.buffer {buffer.get_handle()},
		.offset {offset},
		.range {range}
	};
	assert(bufferInfo.buffer != VK_NULL_HANDLE);

	VkWriteDescriptorSet write{
		.sType {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
		.pNext {nullptr},
		.dstSet {VK_NULL_HANDLE},
		.dstBinding {DescriptorSetLayout::storageBufferBinding},
		.dstArrayElement {0}, //Todo
		.descriptorCount {1},
		.descriptorType {DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::storageBufferBinding)},
		.pImageInfo {nullptr},
		.pBufferInfo {&bufferInfo},
		.pTexelBufferView {nullptr}
	};

	r_device.vkCmdPushDescriptorSetKHR(m_cmd, m_bindPoint, m_layout, firstSet, 1, &write);
}

void PipelineBind::bind_descriptor_set(uint32_t firstSet, const DescriptorSet& set) const noexcept
{
	//No plans to support dynamic offsets
	r_device.vkCmdBindDescriptorSets(m_cmd, m_bindPoint, m_layout, firstSet, 1, &set.get_handle(), 0, nullptr);
}

PipelineBind::PipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) noexcept :
	r_device(device),
	m_cmd(cmd),
	m_layout(layout),
	m_bindPoint(bindPoint)
{
	assert(m_cmd != VK_NULL_HANDLE);
	assert(m_layout != VK_NULL_HANDLE);
	assert(m_bindPoint == VK_PIPELINE_BIND_POINT_GRAPHICS || 
		m_bindPoint == VK_PIPELINE_BIND_POINT_COMPUTE ||
		m_bindPoint == VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}



void ComputePipelineBind::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) const noexcept 
{
	r_device.vkCmdDispatch(m_cmd, groupCountX, groupCountY, groupCountZ);
}

ComputePipelineBind::ComputePipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	PipelineBind(device, cmd, layout, VK_PIPELINE_BIND_POINT_COMPUTE)
{
}

void GraphicsPipelineBind::set_viewport(uint16_t width, uint16_t height, float minDepth, float maxDepth) const noexcept
{
	VkViewport viewport{
		.x {0.f},
		.y {0.f},
		.width {static_cast<float>(width)},
		.height {static_cast<float>(height)},
		.minDepth {minDepth},
		.maxDepth {maxDepth}
	};
	r_device.vkCmdSetViewport(m_cmd, 0, 1, &viewport);
}

void GraphicsPipelineBind::set_scissor(uint16_t width, uint16_t height, uint16_t x, uint16_t y) const noexcept
{
	VkRect2D scissor{
		.offset {
			.x {x},
			.y {y}
		},
		.extent {
			.width {width},
			.height {height}
		}
	};

	r_device.vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

GraphicsPipelineBind::GraphicsPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	PipelineBind(device, cmd, layout, VK_PIPELINE_BIND_POINT_GRAPHICS)
{
}

VertexPipelineBind::VertexPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	GraphicsPipelineBind(device, cmd, layout)
{
}

MeshPipelineBind::MeshPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	GraphicsPipelineBind(device, cmd, layout)
{
}

RayTracingPipelineBind::RayTracingPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	PipelineBind(device, cmd, layout, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR)
{
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_queue(other.r_queue),
	m_state(other.m_state)
{
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
	if(this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		assert(std::addressof(r_queue) == std::addressof(other.r_queue));
		std::swap(m_handle, other.m_handle);
		std::swap(m_state, other.m_state);
	}
	return *this;
}

std::expected<void, Error> CommandBuffer::begin(const bool oneTimeSubmit) noexcept
{
	assert(m_state == State::Initial);
	if(m_state != State::Initial) //We assume that we only use completely reset commandPools
		return std::unexpected{ VK_ERROR_UNKNOWN };

	const VkCommandBufferBeginInfo beginInfo{
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO},
		.pNext {nullptr}, //Only currently supported is DeviceGroups, which are kinda deprecated anyways
		.flags {oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : static_cast<VkCommandBufferUsageFlags>(0)},
		.pInheritanceInfo {nullptr} //Secondary command buffers are not that useful so currently skip support
	};

	if (const auto result = r_device.vkBeginCommandBuffer(m_handle, &beginInfo); result != VK_SUCCESS)
		return std::unexpected{ result };
	m_state = State::Recording;
	return {};
}

std::expected<void, Error> CommandBuffer::end() noexcept
{
	assert(m_state == State::Recording);
	if (m_state != State::Recording)
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (const auto result = r_device.vkEndCommandBuffer(m_handle); result != VK_SUCCESS)
		return std::unexpected{ result };
	m_state = State::Executable;
	return {};
}


//void CommandBuffer::begin_rendering(const VkRenderingInfo& info) noexcept
//{
//	r_device.vkCmdBeginRendering(m_handle, &info);
//}
//
//void CommandBuffer::end_rendering() noexcept
//{
//	r_device.vkCmdEndRendering(m_handle);
//}

VertexPipelineBind CommandBuffer::bind_vertex_pipeline(const VertexShaderGraphicsPipeline& pipeline, GraphicsPipeline::DynamicStates dynamicDefaultsMask) noexcept
{
	r_device.vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
	VertexPipelineBind bind{ r_device, m_handle, pipeline.get_layout().get_handle() };
	const auto& pipelineState = pipeline.get_pipeline_state();
	auto dynamicState = pipelineState.dynamicState & dynamicDefaultsMask;

	dynamicState.for_each([&](GraphicsPipeline::DynamicState state){
		switch (state) {
			using enum GraphicsPipeline::DynamicState;
			case Viewport:
				bind.set_viewport(pipelineState.viewport.width, pipelineState.viewport.height);
				break;
			case Scissor:
				bind.set_scissor(pipelineState.viewport.width, pipelineState.viewport.height, 0, 0);
				break;
			default:
				assert(false && "TODO");
		}
		});
	
	return bind;
}

ComputePipelineBind CommandBuffer::bind_pipeline(const ComputePipeline& pipeline) noexcept
{
	assert(r_queue.get_type() == Queue::Type::Compute ||
		r_queue.get_type() == Queue::Type::Graphics);
	assert(m_state == State::Recording);
	r_device.vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.get_handle());
	return ComputePipelineBind{r_device, m_handle, pipeline.get_layout().get_handle()};
}

void CommandBuffer::begin_region(const char* name, const float* color) noexcept
{
	assert(vkCmdBeginDebugUtilsLabelEXT);
	VkDebugUtilsLabelEXT label{
		.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {name}
	};
	if (color) {
		for (uint32_t i = 0; i < 4; i++)
			label.color[i] = color[i];
	}
	else {
		for (uint32_t i = 0; i < 4; i++)
			label.color[i] = 1.0f;
	}
	vkCmdBeginDebugUtilsLabelEXT(m_handle, &label);
}

void CommandBuffer::end_region() noexcept
{
	assert(vkCmdEndDebugUtilsLabelEXT);
	vkCmdEndDebugUtilsLabelEXT(m_handle);
}

CommandBuffer::~CommandBuffer() noexcept
{

}

Queue::Type CommandBuffer::get_type() const noexcept
{
	return r_queue.get_type();
}

CommandBuffer::CommandBuffer(const LogicalDeviceWrapper& device,
                             const VkCommandBuffer handle, 
								Queue& queue) noexcept :
	Object(device, handle),
	r_queue(queue),
	m_state(State::Initial)
{
}
