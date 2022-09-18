#include "CommandBuffer.h"

#include <stdexcept>

#include "Instance.h"
#include "LogicalDevice.h"
#include "Image.h"
#include "Buffer.h"
#include "AccelerationStructure.h"
#include "Utility/Exceptions.h"

vulkan::CommandBuffer::CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, CommandBufferType type, uint32_t threadIdx) :
	VulkanObject(parent, handle),
	m_threadIdx(threadIdx),
	m_type(type)
{
	assert(m_handle);
	if constexpr (debug) {
#ifdef DEBUGSUBMISSIONS
		std::cout << "Created commandbuffer with handle: " << handle << '\n';
#endif
	}
}

void vulkan::CommandBuffer::begin_rendering(const VkRenderingInfo& info)
{
	vkCmdBeginRendering(m_handle, &info);
}

void vulkan::CommandBuffer::end_rendering()
{
	vkCmdEndRendering(m_handle);
}

vulkan::GraphicsPipelineBind vulkan::CommandBuffer::bind_graphics_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, *pipeline);

	const auto& dynamicState = pipeline->get_dynamic_state();
	//if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::ViewportWithCount))
	//	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
	//if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::ScissorWithCount))
	//	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
	//if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::LineWidth))
	//	dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	//if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBias))
	//	dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilCompareMask)) {
		vkCmdSetStencilCompareMask(m_handle, VK_STENCIL_FACE_FRONT_BIT, dynamicState.stencil_front_compare_mask);
		vkCmdSetStencilCompareMask(m_handle, VK_STENCIL_FACE_BACK_BIT, dynamicState.stencil_back_compare_mask);
	}
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilReference)) {
		vkCmdSetStencilReference(m_handle, VK_STENCIL_FACE_FRONT_BIT, dynamicState.stencil_front_reference);
		vkCmdSetStencilReference(m_handle, VK_STENCIL_FACE_BACK_BIT, dynamicState.stencil_back_reference);
	}
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilWriteMask)) {
		vkCmdSetStencilWriteMask(m_handle, VK_STENCIL_FACE_FRONT_BIT, dynamicState.stencil_front_write_mask);
		vkCmdSetStencilWriteMask(m_handle, VK_STENCIL_FACE_BACK_BIT, dynamicState.stencil_back_write_mask);
	}
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::CullMode))
		vkCmdSetCullMode(m_handle, dynamicState.cull_mode);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::FrontFace))
		vkCmdSetFrontFace(m_handle, dynamicState.front_face);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::PrimitiveTopology))
		vkCmdSetPrimitiveTopology(m_handle, dynamicState.primitive_topology);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthTestEnabled))
		vkCmdSetDepthTestEnable(m_handle, dynamicState.depth_test_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthWriteEnabled))
		vkCmdSetDepthWriteEnable(m_handle, dynamicState.depth_write_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthCompareOp))
		vkCmdSetDepthCompareOp(m_handle, dynamicState.depth_compare_op);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBoundsTestEnabled))
		vkCmdSetDepthBoundsTestEnable(m_handle, dynamicState.depth_bounds_test_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilTestEnabled))
		vkCmdSetStencilTestEnable(m_handle, dynamicState.stencil_test_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilOp)) {
		vkCmdSetStencilOp(m_handle, VK_STENCIL_FACE_FRONT_BIT, dynamicState.stencil_front_fail, dynamicState.stencil_front_pass, dynamicState.stencil_front_depth_fail, dynamicState.stencil_front_compare_op);
		vkCmdSetStencilOp(m_handle, VK_STENCIL_FACE_BACK_BIT, dynamicState.stencil_back_fail, dynamicState.stencil_back_pass, dynamicState.stencil_back_depth_fail, dynamicState.stencil_back_compare_op);
	}
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBiasEnabled))
		vkCmdSetDepthBiasEnable(m_handle, dynamicState.depth_bias_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::PrimitiveRestartEnabled))
		vkCmdSetPrimitiveRestartEnable(m_handle, dynamicState.primitive_restart_enable);
	if (dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::RasterizerDiscardEnabled))
		vkCmdSetRasterizerDiscardEnable(m_handle, dynamicState.rasterizer_discard_enable);

	auto set = r_device.get_bindless_set().get_set();
	vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_layout(), 0, 1, &set, 0, nullptr);

	return { m_handle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_GRAPHICS };
}

vulkan::ComputePipelineBind vulkan::CommandBuffer::bind_compute_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, *pipeline);

	auto set = r_device.get_bindless_set().get_set();
	vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->get_layout(), 0, 1, &set, 0, nullptr);

	return { m_handle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_COMPUTE };
}

vulkan::RaytracingPipelineBind vulkan::CommandBuffer::bind_raytracing_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, *pipeline);
	auto set = r_device.get_bindless_set().get_set();
	vkCmdBindDescriptorSets(m_handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->get_layout(), 0, 1, &set, 0, nullptr);
	return { m_handle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR };
}

void vulkan::CommandBuffer::copy_buffer(const Buffer& dst, const Buffer& src, VkDeviceSize dstOffset, VkDeviceSize srcOffset, VkDeviceSize size)
{
	VkBufferCopy copy{
		.srcOffset = srcOffset,
		.dstOffset = dstOffset,
		.size = size,
	};
	copy_buffer(dst, src, &copy, 1);
}

void vulkan::CommandBuffer::copy_buffer(const Buffer& dst, const Buffer& src, const VkBufferCopy* copies, uint32_t copyCount)
{
	vkCmdCopyBuffer(m_handle, src.get_handle(), dst.get_handle(), copyCount, copies);
}

void vulkan::CommandBuffer::copy_buffer(const Buffer& dst, const Buffer& src)
{
	copy_buffer(dst, src, 0, 0, dst.get_info().size);
}

void vulkan::CommandBuffer::blit_image(const Image& dst, const Image& src, const VkOffset3D& dstOffset, const VkOffset3D& dstExtent,
	const VkOffset3D& srcOffset, const VkOffset3D& srcExtent, uint32_t dstLevel,
	uint32_t srcLevel, uint32_t dstLayer, uint32_t srcLayer, uint32_t layerCount, VkFilter filter)
{
	const auto addOffset = [](const VkOffset3D& a, const VkOffset3D& b) -> VkOffset3D { return { a.x + b.x, a.y + b.y, a.z + b.z }; };
	for (uint32_t layer = 0; layer < layerCount; layer++) {
		const VkImageBlit blit{
			.srcSubresource {
				.aspectMask = ImageInfo::format_to_aspect_mask(src.get_info().format),
				.mipLevel = srcLevel,
				.baseArrayLayer = srcLayer + layer,
				.layerCount = 1
			},
			.srcOffsets {
				srcOffset,
				addOffset(srcOffset, srcExtent)
			},
			.dstSubresource {
				.aspectMask = ImageInfo::format_to_aspect_mask(dst.get_info().format),
				.mipLevel = dstLevel,
				.baseArrayLayer = dstLayer + layer,
				.layerCount = 1
			},
			.dstOffsets {
				dstOffset,
				addOffset(dstOffset, dstExtent)
			},
		};
		vkCmdBlitImage(m_handle, src.get_handle(), src.is_optimal() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL, 
			dst.get_handle(), dst.is_optimal() ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
			1, &blit, filter);
	}
}

void vulkan::CommandBuffer::copy_image(const Image& src, const Image& dst, VkImageLayout srcLayout, VkImageLayout dstLayout, uint32_t mipLevel)
{
	VkImageCopy region{
		.srcSubresource 
		{
			.aspectMask {ImageInfo::format_to_aspect_mask(src.get_format())},
			.mipLevel {mipLevel},
			.baseArrayLayer {0},
			.layerCount {src.get_info().arrayLayers},
		},
		.srcOffset {},
		.dstSubresource
		{
			.aspectMask {ImageInfo::format_to_aspect_mask(dst.get_format())},
			.mipLevel {mipLevel},
			.baseArrayLayer {0},
			.layerCount {src.get_info().arrayLayers},
		},
		.dstOffset {},
		.extent 
		{
			.width {src.get_width(mipLevel)},
			.height {src.get_height(mipLevel)},
			.depth {src.get_depth(mipLevel)},
		},
	};
	vkCmdCopyImage(m_handle, src.get_handle(), srcLayout, dst.get_handle(), dstLayout, 1, &region);
}


void vulkan::CommandBuffer::generate_mips(const Image& image)
{
	auto& info = image.get_info();
	VkOffset3D size{.x = static_cast<int32_t>(info.width), .y = static_cast<int32_t>(info.height), .z = static_cast<int32_t>(info.depth)};
	const VkOffset3D origin{ 0, 0,0 };
	assert(image.is_optimal());
	VkImageMemoryBarrier imageBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.get_handle(),
		.subresourceRange {
			.aspectMask = ImageInfo::format_to_aspect_mask(image.get_format()),
			.levelCount = 1,
			.layerCount = info.arrayLayers
		},
	};
	for (uint32_t level = 1; level < info.mipLevels; level++) {
		VkOffset3D srcSize = size;
		size.x = Math::max(size.x / 2, 1);
		size.y = Math::max(size.y / 2, 1);
		size.z = Math::max(size.z / 2, 1);
		blit_image(image, image, origin, size, origin, srcSize, level, level - 1, 0, 0, info.arrayLayers, VK_FILTER_LINEAR);
		imageBarrier.subresourceRange.baseMipLevel = level;
		barrier(VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr, 1, &imageBarrier);
	}
}

void vulkan::CommandBuffer::copy_buffer_to_image(const Image& image, const Buffer& buffer, uint32_t blitCounts, const VkBufferImageCopy* blits)
{
	vkCmdCopyBufferToImage(m_handle, buffer.get_handle(), image.get_handle(),
		image.is_optimal() ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL , blitCounts, blits);
}

void vulkan::CommandBuffer::copy_acceleration_structure(const AccelerationStructure& src, const AccelerationStructure& dst, bool compact)
{
	assert(src.is_compactable() == compact && "Tried to compact a non compactable acceleration structure");
	VkCopyAccelerationStructureInfoKHR info{
		.sType = VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR,
		.pNext = nullptr,
		.src = src.get_handle(),
		.dst = dst.get_handle(),
		.mode = src.is_compactable() == compact ? VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR : VK_COPY_ACCELERATION_STRUCTURE_MODE_CLONE_KHR,
	};
	vkCmdCopyAccelerationStructureKHR(m_handle, &info);
}

void vulkan::CommandBuffer::mip_barrier(const Image& image, VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool needBarrier)
{
	assert(image.get_info().mipLevels > 1);
	std::array<VkImageMemoryBarrier, 2> barriers {};
	barriers[0] = barriers[1] = VkImageMemoryBarrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = srcAccess,
		.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
		.oldLayout = layout,
		.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.get_handle(),
		.subresourceRange {
			.aspectMask = ImageInfo::format_to_aspect_mask(image.get_format()),
			.baseMipLevel = 0,
			.levelCount = 1,
			.layerCount = image.get_info().arrayLayers,
		}
	};
	barriers[1].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barriers[1].newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barriers[1].srcAccessMask = 0;
	barriers[1].dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barriers[1].subresourceRange.baseMipLevel = 1;
	barriers[1].subresourceRange.levelCount = image.get_info().mipLevels - 1;

	barrier(srcStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, nullptr, 0, nullptr,
		needBarrier ? 2 : 1, needBarrier? barriers.data(): &barriers[1]);
}

void vulkan::CommandBuffer::barrier(VkPipelineStageFlags srcStages, VkPipelineStageFlags dstStages, uint32_t barrierCount,
	const VkMemoryBarrier* globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier* bufferBarriers,
	uint32_t imageBarrierCounts, const VkImageMemoryBarrier* imageBarriers)
{
	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers != nullptr));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].sType == VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].buffer != VK_NULL_HANDLE));
	vkCmdPipelineBarrier(m_handle, srcStages, dstStages, 0, barrierCount,
		globals, bufferBarrierCounts, bufferBarriers, imageBarrierCounts, imageBarriers);
}

void vulkan::CommandBuffer::barrier2(uint32_t barrierCount, const VkMemoryBarrier2* globals,
	uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers,
	uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{
	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers != nullptr));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].sType == VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].buffer != VK_NULL_HANDLE));
	assert(!(barrierCount != 0) || (globals != nullptr));
	assert(!(barrierCount != 0) || (globals[0].sType == VK_STRUCTURE_TYPE_MEMORY_BARRIER_2));
	//for (auto a = 0; a < imageBarrierCounts; ++a)
	//	assert(!(imageBarriers[a].dstStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < imageBarrierCounts; ++a)
	//	assert(!(imageBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < bufferBarrierCounts; ++a)
	//	assert(!(bufferBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < bufferBarrierCounts; ++a)
	//	assert(!(bufferBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < barrierCount; ++a)
	//	assert(!(globals[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < barrierCount; ++a)
	//	assert(!(globals[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	VkDependencyInfo dependencyInfo{
		.sType {VK_STRUCTURE_TYPE_DEPENDENCY_INFO},
		.pNext {nullptr},
		.dependencyFlags {dependencyFlags},
		.memoryBarrierCount {barrierCount},
		.pMemoryBarriers {globals},
		.bufferMemoryBarrierCount {bufferBarrierCounts},
		.pBufferMemoryBarriers {bufferBarriers},
		.imageMemoryBarrierCount {imageBarrierCounts},
		.pImageMemoryBarriers {imageBarriers}
	};
	vkCmdPipelineBarrier2(m_handle, &dependencyInfo);
}
void vulkan::CommandBuffer::barrier2(uint32_t barrierCount, const VkMemoryBarrier2* globals, VkDependencyFlags dependencyFlags)
{
	barrier2(barrierCount, globals, 0, nullptr, 0, nullptr, dependencyFlags);
}
void vulkan::CommandBuffer::barrier2(uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags)
{
	barrier2(0, nullptr, bufferBarrierCounts, bufferBarriers, 0, nullptr, dependencyFlags);
}

void vulkan::CommandBuffer::barrier2(uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{
	barrier2(0, nullptr, 0, nullptr, imageBarrierCounts, imageBarriers, dependencyFlags);
}

void vulkan::CommandBuffer::reset_event2(VkEvent event, VkPipelineStageFlags2 stages)
{
	vkCmdResetEvent2(m_handle, event, stages);
}

void vulkan::CommandBuffer::set_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{

	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers != nullptr));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].sType == VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].buffer != VK_NULL_HANDLE));
	assert(!(barrierCount != 0) || (globals != nullptr));
	assert(!(barrierCount != 0) || (globals[0].sType == VK_STRUCTURE_TYPE_MEMORY_BARRIER_2));
	VkDependencyInfo dependencyInfo{
		.sType {VK_STRUCTURE_TYPE_DEPENDENCY_INFO},
		.pNext {nullptr},
		.dependencyFlags {dependencyFlags},
		.memoryBarrierCount {barrierCount},
		.pMemoryBarriers {globals},
		.bufferMemoryBarrierCount {bufferBarrierCounts},
		.pBufferMemoryBarriers {bufferBarriers},
		.imageMemoryBarrierCount {imageBarrierCounts},
		.pImageMemoryBarriers {imageBarriers}
	};
	vkCmdSetEvent2(m_handle, event, &dependencyInfo);
}

void vulkan::CommandBuffer::set_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, VkDependencyFlags dependencyFlags)
{
	set_event2(event, barrierCount, globals, 0, nullptr, 0, nullptr, dependencyFlags);
}

void vulkan::CommandBuffer::set_event2(VkEvent event, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags)
{
	set_event2(event, 0, nullptr, bufferBarrierCounts, bufferBarriers, 0, nullptr, dependencyFlags);
}

void vulkan::CommandBuffer::set_event2(VkEvent event, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{
	set_event2(event, 0, nullptr, 0, nullptr, imageBarrierCounts, imageBarriers, dependencyFlags);
}

void vulkan::CommandBuffer::wait_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{
	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers != nullptr));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].sType == VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2));
	assert(!(bufferBarrierCounts != 0) || (bufferBarriers[0].buffer != VK_NULL_HANDLE));
	assert(!(barrierCount != 0) || (globals != nullptr));
	assert(!(barrierCount != 0) || (globals[0].sType == VK_STRUCTURE_TYPE_MEMORY_BARRIER_2));
	//for (auto a = 0; a < imageBarrierCounts; ++a)
	//	assert(!(imageBarriers[a].dstStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < imageBarrierCounts; ++a)
	//	assert(!(imageBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < bufferBarrierCounts; ++a)
	//	assert(!(bufferBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < bufferBarrierCounts; ++a)
	//	assert(!(bufferBarriers[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < barrierCount; ++a)
	//	assert(!(globals[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	//for (auto a = 0; a < barrierCount; ++a)
	//	assert(!(globals[a].srcStageMask & VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT));
	VkDependencyInfo dependencyInfo{
		.sType {VK_STRUCTURE_TYPE_DEPENDENCY_INFO},
		.pNext {nullptr},
		.dependencyFlags {dependencyFlags},
		.memoryBarrierCount {barrierCount},
		.pMemoryBarriers {globals},
		.bufferMemoryBarrierCount {bufferBarrierCounts},
		.pBufferMemoryBarriers {bufferBarriers},
		.imageMemoryBarrierCount {imageBarrierCounts},
		.pImageMemoryBarriers {imageBarriers}
	};
	wait_events2(1, &event, &dependencyInfo);
}

void vulkan::CommandBuffer::wait_event2(VkEvent event, uint32_t barrierCount, const VkMemoryBarrier2* globals, VkDependencyFlags dependencyFlags)
{
	wait_event2(event, barrierCount, globals, 0, nullptr, 0, nullptr, dependencyFlags);
}

void vulkan::CommandBuffer::wait_event2(VkEvent event, uint32_t bufferBarrierCounts, const VkBufferMemoryBarrier2* bufferBarriers, VkDependencyFlags dependencyFlags)
{
	wait_event2(event, 0, nullptr, bufferBarrierCounts, bufferBarriers, 0, nullptr, dependencyFlags);
}

void vulkan::CommandBuffer::wait_event2(VkEvent event, uint32_t imageBarrierCounts, const VkImageMemoryBarrier2* imageBarriers, VkDependencyFlags dependencyFlags)
{
	wait_event2(event, 0, nullptr, 0, nullptr, imageBarrierCounts, imageBarriers, dependencyFlags);
}

void vulkan::CommandBuffer::wait_events2(uint32_t eventCount, const VkEvent* event, const VkDependencyInfo* dependencyInfo)
{
	assert(eventCount);
	assert(event);
	assert(dependencyInfo);
	vkCmdWaitEvents2(m_handle, eventCount, event, dependencyInfo);
}

void vulkan::CommandBuffer::barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess)
{
	VkMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = srcAccess,
		.dstAccessMask = dstAccess
	};
	vkCmdPipelineBarrier(m_handle, srcStages, dstStages, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void vulkan::CommandBuffer::image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags, VkPipelineStageFlags dstStages, VkAccessFlags dstAccessFlags)
{
	VkImageMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		.srcAccessMask = srcAccessFlags,
		.dstAccessMask = dstAccessFlags,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image.get_handle(),
		.subresourceRange {
			.aspectMask = ImageInfo::format_to_aspect_mask(image.get_info().format),
			.baseMipLevel = 0,
			.levelCount = image.get_info().mipLevels,
			.baseArrayLayer = 0,
			.layerCount = image.get_info().arrayLayers
		}
	};
	assert(barrier.image != VK_NULL_HANDLE);
	vkCmdPipelineBarrier(m_handle, srcStages, dstStages, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vulkan::CommandBuffer::clear_color_image(const Image& image, VkImageLayout layout, const VkClearColorValue* clearColor)
{
	VkImageSubresourceRange range{
		.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},
		.baseMipLevel {0},
		.levelCount {VK_REMAINING_MIP_LEVELS},
		.baseArrayLayer {0},
		.layerCount {VK_REMAINING_ARRAY_LAYERS},
	};
	vkCmdClearColorImage(m_handle, image.get_handle(), layout, clearColor, 1, &range);
}

void vulkan::CommandBuffer::clear_depth_image(const Image& image, VkImageLayout layout, const VkClearDepthStencilValue* clearColor)
{
	assert(false);
	VkImageSubresourceRange range{
		.aspectMask {vulkan::ImageInfo::format_to_aspect_mask(image.get_format())},
		.baseMipLevel {0},
		.levelCount {VK_REMAINING_MIP_LEVELS},
		.baseArrayLayer {0},
		.layerCount {VK_REMAINING_ARRAY_LAYERS},
	};
	vkCmdClearDepthStencilImage(m_handle, image.get_handle(), layout, clearColor, 1, &range);
}

bool vulkan::CommandBuffer::swapchain_touched() const noexcept
{
	return m_swapchainTouched;
}

void vulkan::CommandBuffer::touch_swapchain() noexcept
{
	m_swapchainTouched = true;
}

void vulkan::CommandBuffer::end()
{
	if (auto result = vkEndCommandBuffer(m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}

void vulkan::CommandBuffer::begin_region(const char* name, const float* color)
{
	if constexpr (debugMarkers) {
		if (r_device.get_supported_extensions().debug_utils) {
			VkDebugUtilsLabelEXT label{
				.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
				.pLabelName = name
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
		else if (r_device.get_supported_extensions().debug_marker) {
			VkDebugMarkerMarkerInfoEXT marker{
				.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT,
				.pMarkerName = name
			};
			if (color) {
				for (uint32_t i = 0; i < 4; i++)
					marker.color[i] = color[i];
			}
			else {
				for (uint32_t i = 0; i < 4; i++)
					marker.color[i] = 1.0f;
			}
			vkCmdDebugMarkerBeginEXT(m_handle, &marker);
		}
	}
}

void vulkan::CommandBuffer::end_region()
{
	if constexpr (debugMarkers) {
		if (r_device.get_supported_extensions().debug_utils) {
			vkCmdEndDebugUtilsLabelEXT(m_handle);
		}
		else if (r_device.get_supported_extensions().debug_marker) {
			vkCmdDebugMarkerEndEXT(m_handle);
		}
	}
}

vulkan::CommandBufferType vulkan::CommandBuffer::get_type() const noexcept {
	return m_type;
}
