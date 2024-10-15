// ReSharper disable CppMemberFunctionMayBeConst
// Functions are technically const but semantically they modify something
module;

#include <bit>
#include <cassert>
#include <expected>
#include <utility>

#include "volk.h"

module NYANVulkan;
import NYANData;
import :DescriptorSet;
import :Pipeline;

using namespace nyan::vulkan;

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

	ptr_device->vkCmdPushDescriptorSetKHR(m_cmd, m_bindPoint, m_layout, firstSet, 1, &write);
}

void PipelineBind::bind_descriptor_set(uint32_t firstSet, const DescriptorSet& set) const noexcept
{
	//No plans to support dynamic offsets
	ptr_device->vkCmdBindDescriptorSets(m_cmd, m_bindPoint, m_layout, firstSet, 1, &set.get_handle(), 0, nullptr);
}

PipelineBind::PipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) noexcept :
	ptr_device(&device),
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
	ptr_device->vkCmdDispatch(m_cmd, groupCountX, groupCountY, groupCountZ);
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
	ptr_device->vkCmdSetViewport(m_cmd, 0, 1, &viewport);
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

	ptr_device->vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

void GraphicsPipelineBind::begin_render_pass(const RenderPass& renderPass, const Framebuffer& framebuffer) const noexcept
{
	static_assert(sizeof(VkClearValue) == sizeof(ClearValue));
	auto clearValues = renderPass.get_clear_values();
	VkRenderPassBeginInfo beginInfo{
		.sType {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO},
		.pNext {nullptr},
		.renderPass {renderPass.get_handle()},
		.framebuffer {framebuffer.get_handle()},
		.renderArea {.offset {0, 0}, .extent {.width {framebuffer.get_width()}, .height{framebuffer.get_height()}}},
		.clearValueCount {static_cast<uint32_t>(clearValues.size())},
		.pClearValues {reinterpret_cast<const VkClearValue*>(clearValues.data())}
	};
	ptr_device->vkCmdBeginRenderPass(m_cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void GraphicsPipelineBind::end_render_pass() const noexcept
{
	ptr_device->vkCmdEndRenderPass(m_cmd);
}

void GraphicsPipelineBind::next_subpass() const noexcept
{
	ptr_device->vkCmdNextSubpass(m_cmd, VK_SUBPASS_CONTENTS_INLINE);
}

void GraphicsPipelineBind::begin_rendering(const RenderingInfo& _renderingInfo) const noexcept
{
	nyan::StaticVector< VkRenderingAttachmentInfo, maxOutputAttachments> colorAttachments;

	uint32_t width{ 0 };
	uint32_t height{ 0 };
	uint32_t layers{ 0 };

	for (const auto& attachment : _renderingInfo.colorAttachments) {
		if (auto localWidth = attachment.image.get_width(); width != 0 && width != localWidth)
			assert(false);
		else
			width = localWidth;

		if (auto localHeight = attachment.image.get_height(); height != 0 && height != localHeight)
			assert(false);
		else
			height = localHeight;

		if (auto localLayers = attachment.image.get_layers(); layers != 0 && layers != localLayers)
			assert(false);
		else
			layers = localLayers;
		assert(attachment.layout != Layout::Undefined);
		nyan::ignore = colorAttachments.push_back(VkRenderingAttachmentInfo{
				.sType{VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO},
				.pNext{nullptr},
				.imageView{attachment.image.get_image_view(0).get_handle()},
				.imageLayout{static_cast<VkImageLayout>(convert_layout(attachment.layout))},
				.resolveMode{static_cast<VkResolveModeFlagBits>(attachment.resolveMode)},
				.resolveImageView{attachment.resolveImage? attachment.resolveImage->get_image_view(0).get_handle() : VK_NULL_HANDLE},
				.resolveImageLayout{attachment.resolveImage ? VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED },
				.loadOp{static_cast<VkAttachmentLoadOp>(attachment.loadOp)},
				.storeOp{static_cast<VkAttachmentStoreOp>(attachment.storeOp)},
				.clearValue{static_cast<VkClearValue>(attachment.clearValue)},
			});
	}

	VkRenderingAttachmentInfo depthAttachment;
	if (_renderingInfo.depthAttachment) {
		const auto& attachment = *_renderingInfo.depthAttachment;
		depthAttachment = VkRenderingAttachmentInfo{
			.sType{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO },
			.pNext{ nullptr },
			.imageView{ attachment.image.get_image_view(0).get_handle() },
			.imageLayout{ static_cast<VkImageLayout>(convert_layout(attachment.layout)) },
			.resolveMode{ static_cast<VkResolveModeFlagBits>(attachment.resolveMode) },
			.resolveImageView{ attachment.resolveImage ? attachment.resolveImage->get_image_view(0).get_handle() : VK_NULL_HANDLE },
			.resolveImageLayout{ attachment.resolveImage ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED },
			.loadOp{ static_cast<VkAttachmentLoadOp>(attachment.loadOp) },
			.storeOp{ static_cast<VkAttachmentStoreOp>(attachment.storeOp) },
			.clearValue{ static_cast<VkClearValue>(attachment.clearValue )},
		};
	}
	VkRenderingAttachmentInfo stencilAttachment;
	if (_renderingInfo.stencilAttachment) {
		const auto& attachment = *_renderingInfo.stencilAttachment;
		depthAttachment = VkRenderingAttachmentInfo{
			.sType{ VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO },
			.pNext{ nullptr },
			.imageView{ attachment.image.get_image_view(1).get_handle() },
			.imageLayout{ static_cast<VkImageLayout>(convert_layout(attachment.layout)) },
			.resolveMode{ static_cast<VkResolveModeFlagBits>(attachment.resolveMode) },
			.resolveImageView{ attachment.resolveImage ? attachment.resolveImage->get_image_view(1).get_handle() : VK_NULL_HANDLE },
			.resolveImageLayout{ attachment.resolveImage ? VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL : VK_IMAGE_LAYOUT_UNDEFINED },
			.loadOp{ static_cast<VkAttachmentLoadOp>(attachment.loadOp) },
			.storeOp{ static_cast<VkAttachmentStoreOp>(attachment.storeOp) },
			.clearValue{static_cast<VkClearValue>(attachment.clearValue)},
		};
	}

	VkRenderingInfo renderingInfo{
		.sType {VK_STRUCTURE_TYPE_RENDERING_INFO},
		.pNext {nullptr},
		.flags {0},
		.renderArea {.offset {0, 0},.extent {.width {width}, .height {height}}},
		.layerCount {layers},
		.viewMask {0},
		.colorAttachmentCount {static_cast<uint32_t>(colorAttachments.size())},
		.pColorAttachments {colorAttachments.data()},
		.pDepthAttachment {_renderingInfo.stencilAttachment ? &depthAttachment : nullptr},
		.pStencilAttachment {_renderingInfo.stencilAttachment ? &stencilAttachment : nullptr}
	};
	ptr_device->vkCmdBeginRendering(m_cmd, &renderingInfo);
}

void GraphicsPipelineBind::end_rendering() const noexcept
{
	ptr_device->vkCmdEndRendering(m_cmd);
}

GraphicsPipelineBind::GraphicsPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept :
	PipelineBind(device, cmd, layout, VK_PIPELINE_BIND_POINT_GRAPHICS)
{
}

void VertexPipelineBind::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	ptr_device->vkCmdDraw(m_cmd, vertexCount, instanceCount, firstVertex, firstInstance);
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
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_queue(other.r_queue)
	//m_state(other.m_state)
{
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
	if(this != std::addressof(other))
	{
		assert(ptr_device == other.ptr_device);
		assert(std::addressof(r_queue) == std::addressof(other.r_queue));
		std::swap(m_handle, other.m_handle);
		//std::swap(m_state, other.m_state);
	}
	return *this;
}

std::expected<void, Error> CommandBuffer::begin(const bool oneTimeSubmit) noexcept
{
	//assert(m_state == State::Initial);
	//if(m_state != State::Initial) //We assume that we only use completely reset commandPools
	//	return std::unexpected{ VK_ERROR_UNKNOWN };

	const VkCommandBufferBeginInfo beginInfo{
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO},
		.pNext {nullptr}, //Only currently supported is DeviceGroups, which are kinda deprecated anyways
		.flags {oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : static_cast<VkCommandBufferUsageFlags>(0)},
		.pInheritanceInfo {nullptr} //Secondary command buffers are not that useful so currently skip support
	};

	if (const auto result = ptr_device->vkBeginCommandBuffer(m_handle, &beginInfo); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	//m_state = State::Recording;
	return {};
}

std::expected<void, Error> CommandBuffer::end() noexcept
{
	//assert(m_state == State::Recording);
	//if (m_state != State::Recording)
	//	return std::unexpected{ VK_ERROR_UNKNOWN };

	if (const auto result = ptr_device->vkEndCommandBuffer(m_handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	//m_state = State::Executable;
	return {};
}

void CommandBuffer::barrier2() const noexcept
{
	VkMemoryBarrier2 barrier{
		.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR  },
		.srcAccessMask {VK_ACCESS_2_MEMORY_READ_BIT_KHR |
				   VK_ACCESS_2_MEMORY_WRITE_BIT_KHR },
		.dstStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR   },
		.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT_KHR |
				   VK_ACCESS_2_MEMORY_WRITE_BIT_KHR}
	};
	VkDependencyInfo dependencyInfo{
		.sType {VK_STRUCTURE_TYPE_DEPENDENCY_INFO},
		.pNext {nullptr},
		.dependencyFlags {0},
		.memoryBarrierCount {1},
		.pMemoryBarriers {&barrier},
		.bufferMemoryBarrierCount{0},
		.pBufferMemoryBarriers{nullptr},
		.imageMemoryBarrierCount{0},
		.pImageMemoryBarriers{nullptr},
	};
	ptr_device->vkCmdPipelineBarrier2(m_handle, &dependencyInfo);
}

void CommandBuffer::image_barrier2(const Image& image, Layout srcLayout, Layout dstLayout, Queue::FamilyIndex srcFamilyIndex, Queue::FamilyIndex dstFamilyIndex) const noexcept
{
	VkImageMemoryBarrier2 barrier{
		.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR  },
		.srcAccessMask {VK_ACCESS_2_MEMORY_READ_BIT_KHR |
				   VK_ACCESS_2_MEMORY_WRITE_BIT_KHR },
		.dstStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT_KHR   },
		.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT_KHR |
				   VK_ACCESS_2_MEMORY_WRITE_BIT_KHR},
		.oldLayout {static_cast<VkImageLayout>(convert_layout(srcLayout))},
		.newLayout {static_cast<VkImageLayout>(convert_layout(dstLayout))},
		.srcQueueFamilyIndex {srcFamilyIndex.value},
		.dstQueueFamilyIndex {dstFamilyIndex.value},
		.image {image.get_handle()},
		.subresourceRange {
			.aspectMask {static_cast<VkImageAspectFlags>((image.get_type() == Image::Type::RenderTarget) ? VK_IMAGE_ASPECT_COLOR_BIT : (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) },
			.baseMipLevel {0},
			.levelCount {VK_REMAINING_MIP_LEVELS},
			.baseArrayLayer {0},
			.layerCount{VK_REMAINING_ARRAY_LAYERS}
		},
	};
	VkDependencyInfo dependencyInfo{
		.sType {VK_STRUCTURE_TYPE_DEPENDENCY_INFO},
		.pNext {nullptr},
		.dependencyFlags {0},
		.memoryBarrierCount {0},
		.pMemoryBarriers {nullptr},
		.bufferMemoryBarrierCount{0},
		.pBufferMemoryBarriers{nullptr},
		.imageMemoryBarrierCount{1},
		.pImageMemoryBarriers{&barrier },
	};
	ptr_device->vkCmdPipelineBarrier2(m_handle, &dependencyInfo);
}


//void CommandBuffer::begin_rendering(const VkRenderingInfo& info) noexcept
//{
//	ptr_device->vkCmdBeginRendering(m_handle, &info);
//}
//
//void CommandBuffer::end_rendering() noexcept
//{
//	ptr_device->vkCmdEndRendering(m_handle);
//}

VertexPipelineBind CommandBuffer::bind_pipeline(const VertexShaderGraphicsPipeline& pipeline, GraphicsPipeline::DynamicStates dynamicDefaultsMask) noexcept
{
	ptr_device->vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.get_handle());
	VertexPipelineBind bind{ *ptr_device, m_handle, pipeline.get_layout().get_handle() };
	const auto& pipelineState = pipeline.get_pipeline_state();
	GraphicsPipeline::DynamicStates dynamicState = pipelineState.dynamicState & dynamicDefaultsMask;

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
	//assert(m_state == State::Recording);
	ptr_device->vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline.get_handle());
	return ComputePipelineBind{ *ptr_device, m_handle, pipeline.get_layout().get_handle()};
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

void CommandBuffer::copy_image_to_buffer(const Image& image, Layout layout, const Buffer& buffer) const noexcept
{
	const VkBufferImageCopy imageCopy{
		.bufferOffset {0},
		.bufferRowLength {0},
		.bufferImageHeight {0},
		.imageSubresource {.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},.mipLevel{0}, .baseArrayLayer {0}, .layerCount {image.get_layers()}},
		.imageOffset {0, 0, 0},
		.imageExtent {image.get_width(), image.get_height(), 1},
	};
	ptr_device->vkCmdCopyImageToBuffer(m_handle, image.get_handle(), static_cast<VkImageLayout>(convert_layout(layout)), buffer.get_handle(), 1, &imageCopy);
}

CommandBuffer::CommandBuffer(const LogicalDeviceWrapper& device,
                             const VkCommandBuffer handle, 
								Queue& queue) noexcept :
	Object(device, handle),
	r_queue(queue)
	//m_state(State::Initial)
{
}
