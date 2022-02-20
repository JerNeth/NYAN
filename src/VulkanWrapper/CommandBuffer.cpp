#include "..\..\include\VulkanWrapper\CommandBuffer.h"
#include "..\..\include\VulkanWrapper\CommandBuffer.h"
#include "..\..\include\VulkanWrapper\CommandBuffer.h"
#include "..\..\include\VulkanWrapper\CommandBuffer.h"
#include "..\..\include\VulkanWrapper\CommandBuffer.h"
#include "CommandBuffer.h"
#include "LogicalDevice.h"
vulkan::CommandBuffer::CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type, uint32_t threadIdx) :
	r_device(parent),
	m_vkHandle(handle),
	m_threadIdx(threadIdx),
	m_type(type)
{
	if (type == Type::Generic) {
		m_pipelineState.state = defaultPipelineState;
		if (r_device.get_supported_extensions().extended_dynamic_state) {
			//m_pipelineState.state.dynamic_vertex_input_binding_stride = 1;
			m_pipelineState.state.dynamic_depth_test = 1;
			m_pipelineState.state.dynamic_depth_write = 1;
			m_pipelineState.state.dynamic_cull_mode = 1;
			m_dynamicState.cull_mode = defaultPipelineState.cull_mode;
			m_dynamicState.depth_test = defaultPipelineState.depth_test;
			m_dynamicState.depth_write = defaultPipelineState.depth_write;
		}
	}
	m_pipelineState.program = nullptr;
	m_resourceBindings.bindings = {};
	if constexpr (debug) {
#ifdef DEBUGSUBMISSIONS
		std::cout << "Created commandbuffer with handle: " << handle << '\n';
#endif
	}
}

void vulkan::CommandBuffer::begin_context()
{
	m_invalidFlags.set();
	m_indexState = {};
	m_vertexState.buffers = {};
	m_vertexState.dirty.set();
	m_dirtyDescriptorSets.set();
	m_currentPipeline = VK_NULL_HANDLE;
	m_currentPipelineLayout = nullptr;
	m_pipelineState.program = nullptr;

	
	m_resourceBindings.bindingIds = {};
	m_resourceBindings.samplerIds = {};
}

void vulkan::CommandBuffer::begin_graphics()
{
	m_isCompute = false;
	begin_context();
}

void vulkan::CommandBuffer::begin_compute()
{
	m_isCompute = true;
	begin_context();
}

bool vulkan::CommandBuffer::flush_graphics()
{
	if (!m_pipelineState.program)
		return false;
	assert(m_currentPipelineLayout != nullptr);

	if (m_currentPipeline == VK_NULL_HANDLE)
		m_invalidFlags.set(InvalidFlags::Pipeline);

	if (m_invalidFlags.get_and_clear(InvalidFlags::StaticPipeline, InvalidFlags::Pipeline, InvalidFlags::StaticVertex)) {
		VkPipeline oldPipeline = m_currentPipeline;
		if (!flush_graphics_pipeline())
			return false;
		if (oldPipeline != m_currentPipeline) {
			vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_currentPipeline);
			m_invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	if (m_currentPipeline == VK_NULL_HANDLE)
		return false;
	flush_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS);

	if (m_invalidFlags.get_and_clear(InvalidFlags::PushConstants)) {
		auto& pushConstants = m_currentPipelineLayout->get_shader_layout().pushConstantRange;

		if (pushConstants.stageFlags) {
			assert(pushConstants.offset == 0);
			//assert(false);
			vkCmdPushConstants(m_vkHandle, m_currentPipelineLayout->get_layout(), pushConstants.stageFlags, 0, pushConstants.size, m_resourceBindings.pushConstantData.data());
		}
	}
	if (m_invalidFlags.get_and_clear(InvalidFlags::Viewport))
		vkCmdSetViewport(m_vkHandle, 0, 1, &m_viewport);
	if (m_invalidFlags.get_and_clear(InvalidFlags::Scissor))
		vkCmdSetScissor(m_vkHandle, 0, 1, &m_scissor);
	if (m_pipelineState.state.depth_bias_enable && m_invalidFlags.get_and_clear(InvalidFlags::DepthBias))
		vkCmdSetDepthBias(m_vkHandle,m_dynamicState.depthBias, 0.0f, m_dynamicState.depthBiasSlope);
	if (m_pipelineState.state.stencil_test && m_invalidFlags.get_and_clear(InvalidFlags::Stencil)) {
		vkCmdSetStencilCompareMask(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_dynamicState.frontCompareMask);
		vkCmdSetStencilReference(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_dynamicState.frontReference);
		vkCmdSetStencilWriteMask(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_dynamicState.frontWriteMask);
		vkCmdSetStencilCompareMask(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_dynamicState.backCompareMask);
		vkCmdSetStencilReference(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_dynamicState.backReference);
		vkCmdSetStencilWriteMask(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_dynamicState.backWriteMask);
	}
	if (m_pipelineState.state.dynamic_cull_mode && m_invalidFlags.get_and_clear(InvalidFlags::CullMode)) {
		vkCmdSetCullModeEXT(m_vkHandle, m_dynamicState.cull_mode);
	}
	if (m_pipelineState.state.dynamic_front_face && m_invalidFlags.get_and_clear(InvalidFlags::FrontFace)) {
		vkCmdSetFrontFaceEXT(m_vkHandle, static_cast<VkFrontFace>(m_dynamicState.front_face));
	}
	if (m_pipelineState.state.dynamic_primitive_topology && m_invalidFlags.get_and_clear(InvalidFlags::PrimitiveTopology)) {
		vkCmdSetPrimitiveTopologyEXT(m_vkHandle, static_cast<VkPrimitiveTopology>(m_dynamicState.topology));
	}
	if (m_pipelineState.state.dynamic_depth_test && m_invalidFlags.get_and_clear(InvalidFlags::DepthTest)) {
		vkCmdSetDepthTestEnableEXT(m_vkHandle, m_dynamicState.depth_test);
	}
	if (m_pipelineState.state.dynamic_depth_write && m_invalidFlags.get_and_clear(InvalidFlags::DepthWrite)) {
		vkCmdSetDepthWriteEnableEXT(m_vkHandle, m_dynamicState.depth_write);
	}
	if (m_pipelineState.state.dynamic_depth_compare && m_invalidFlags.get_and_clear(InvalidFlags::DepthCompare)) {
		vkCmdSetDepthCompareOpEXT(m_vkHandle, static_cast<VkCompareOp>(m_dynamicState.depth_compare));
	}
	if (m_pipelineState.state.dynamic_depth_bounds_test && m_invalidFlags.get_and_clear(InvalidFlags::DepthBoundsTest)) {
		vkCmdSetDepthBoundsTestEnableEXT(m_vkHandle, m_dynamicState.depth_bound_test);
	}
	if (m_pipelineState.state.dynamic_stencil_test && m_invalidFlags.get_and_clear(InvalidFlags::StencilTest)) {
		vkCmdSetStencilTestEnableEXT(m_vkHandle, m_dynamicState.stencil_test);
	}
	if (m_pipelineState.state.dynamic_stencil_op && m_invalidFlags.get_and_clear(InvalidFlags::StencilOp)) {
		vkCmdSetStencilOpEXT(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, static_cast<VkStencilOp>(m_dynamicState.stencil_front_fail),
			static_cast<VkStencilOp>(m_dynamicState.stencil_front_pass),static_cast<VkStencilOp>( m_dynamicState.stencil_front_depth_fail), 
			static_cast<VkCompareOp>(m_dynamicState.stencil_front_compare_op));
		vkCmdSetStencilOpEXT(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, static_cast<VkStencilOp>(m_dynamicState.stencil_back_fail),
			static_cast<VkStencilOp>(m_dynamicState.stencil_back_pass), static_cast<VkStencilOp>(m_dynamicState.stencil_back_depth_fail),
			static_cast<VkCompareOp>(m_dynamicState.stencil_back_compare_op));
	}
	bind_vertex_buffers();
	return true;
}

bool vulkan::CommandBuffer::flush_compute()
{
	if (!m_pipelineState.program)
		return false;
	assert(m_currentPipelineLayout != nullptr);

	if (m_currentPipeline == VK_NULL_HANDLE)
		m_invalidFlags.set(InvalidFlags::Pipeline);

	if (m_invalidFlags.get_and_clear(InvalidFlags::Pipeline)) {
		VkPipeline oldPipeline = m_currentPipeline;
		if (!flush_compute_pipeline())
			return false;
		if (oldPipeline != m_currentPipeline) {
			vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_currentPipeline);
			//m_invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	if (m_currentPipeline == VK_NULL_HANDLE)
		return false;
	flush_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE);

	if (m_invalidFlags.get_and_clear(InvalidFlags::PushConstants)) {
		auto& pushConstants = m_currentPipelineLayout->get_shader_layout().pushConstantRange;

		if (pushConstants.stageFlags) {
			assert(pushConstants.offset == 0);
			//assert(false);
			vkCmdPushConstants(m_vkHandle, m_currentPipelineLayout->get_layout(), pushConstants.stageFlags, 0, pushConstants.size, m_resourceBindings.pushConstantData.data());
		}
	}
	return true;
}

bool vulkan::CommandBuffer::flush_ray()
{
	return false;
}

bool vulkan::CommandBuffer::flush_graphics_pipeline()
{
	m_currentPipeline = r_device.request_pipeline(m_pipelineState);
	return m_currentPipeline != VK_NULL_HANDLE;
}

bool vulkan::CommandBuffer::flush_compute_pipeline()
{
	assert(m_pipelineState.program);
	m_currentPipeline = r_device.request_pipeline(*m_pipelineState.program);
	return m_currentPipeline != VK_NULL_HANDLE;
}

bool vulkan::CommandBuffer::flush_ray_pipeline()
{
	return false;
}

void vulkan::CommandBuffer::flush_descriptor_sets(VkPipelineBindPoint bindPoint)
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	
	auto setUpdate = shaderLayout.used & m_dirtyDescriptorSets;
	Utility::for_each_bit(setUpdate, [&](size_t set) {
		flush_descriptor_set(static_cast<uint32_t>(set), bindPoint);
	});
	m_dirtyDescriptorSets &= ~setUpdate;

	m_dirtyDescriptorSetsDynamicOffsets &= ~setUpdate;

	auto dynamicOffsetUpdates = shaderLayout.used & m_dirtyDescriptorSetsDynamicOffsets;
	Utility::for_each_bit(dynamicOffsetUpdates, [&](size_t set) {
		rebind_descriptor_set(static_cast<uint32_t>(set), bindPoint);
	});
	m_dirtyDescriptorSetsDynamicOffsets &= ~dynamicOffsetUpdates;

}

void vulkan::CommandBuffer::flush_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint)
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set];
	std::vector<uint32_t> dynamicOffsets{};
	Utility::Hasher hasher;

	hasher(setLayout.fp);

	size_t bindingSize = 0;
	for (size_t binding = 0; binding < MAX_BINDINGS; ++binding) {
		bindingSize += setLayout.arraySizes[binding];
		if (setLayout.uniformBuffer.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				hasher(m_resourceBindings.bindings[set][binding][i].buffer.range);
				assert(m_resourceBindings.bindings[set][binding][i].buffer.buffer != VK_NULL_HANDLE);
				dynamicOffsets.push_back(static_cast<uint32_t>(m_resourceBindings.dynamicOffsets[set][binding][i]));
			}
		}
		else if (setLayout.storageBuffer.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				hasher(m_resourceBindings.bindings[set][binding][i].buffer.range);
				assert(m_resourceBindings.bindings[set][binding][i].buffer.buffer != VK_NULL_HANDLE);
				dynamicOffsets.push_back(static_cast<uint32_t>(m_resourceBindings.dynamicOffsets[set][binding][i]));
			}
		}
		else if (setLayout.sampledBuffer.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				assert(m_resourceBindings.bindings[set][binding][i].bufferView != VK_NULL_HANDLE);
			}
		}
		else if (setLayout.imageSampler.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				if (!setLayout.immutableSampler.test(binding)) {
					hasher(m_resourceBindings.samplerIds[set][binding][i]);
					assert(m_resourceBindings.bindings[set][binding][i].image.fp.sampler != VK_NULL_HANDLE);
				}
				hasher(m_resourceBindings.bindings[set][binding][i].image.fp.imageLayout);
				assert(m_resourceBindings.bindings[set][binding][i].image.fp.imageView != VK_NULL_HANDLE);
			}
		}
		else if (setLayout.separateImage.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				hasher(m_resourceBindings.bindings[set][binding][i].image.fp.imageLayout);
				assert(m_resourceBindings.bindings[set][binding][i].image.fp.imageView != VK_NULL_HANDLE);
			}
		}
		else if (setLayout.seperateSampler.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.samplerIds[set][binding][i]);
				assert(m_resourceBindings.bindings[set][binding][i].image.fp.sampler != VK_NULL_HANDLE);
			}
		}
		else if (setLayout.storageImage.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				hasher(m_resourceBindings.bindings[set][binding][i].image.fp.imageLayout);
				assert(m_resourceBindings.bindings[set][binding][i].image.fp.imageView != VK_NULL_HANDLE);
			}
		}
		else if (setLayout.inputAttachment.test(binding)) {
			for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
				hasher(m_resourceBindings.bindingIds[set][binding][i]);
				hasher(m_resourceBindings.bindings[set][binding][i].image.fp.imageLayout);
				assert(m_resourceBindings.bindings[set][binding][i].image.fp.imageView != VK_NULL_HANDLE);
			}
		}
	}
	auto hash = hasher();

	auto [descriptorSet, allocated] = m_currentPipelineLayout->get_allocator(set)->find(m_threadIdx, hash);
	if (!allocated) {
		auto updateTemplate = m_currentPipelineLayout->get_update_template(set);

		if (updateTemplate != VK_NULL_HANDLE) {
			//TODO redo
			m_bindingBuffer.clear();
			m_bindingBuffer.resize(bindingSize);
			size_t count = 0;
			for (size_t binding = 0; binding < MAX_BINDINGS; ++binding) {
				for (size_t i = 0; i < setLayout.arraySizes[binding]; i++) {
					m_bindingBuffer[count++] = m_resourceBindings.bindings[set][binding][i];
				}
			}
			vkUpdateDescriptorSetWithTemplate(r_device.get_device(), descriptorSet, updateTemplate, m_bindingBuffer.data());
			//vkCmdPushDescriptorSetWithTemplateKHR(m_vkHandle, updateTemplate, m_currentPipelineLayout->get_layout(),set, m_resourceBindings.bindings[set].data());
		}
		//Fallback maybe?

	}
	vkCmdBindDescriptorSets(m_vkHandle, bindPoint,
		m_currentPipelineLayout->get_layout(), set, 1, &descriptorSet, static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data() );
	m_allocatedDescriptorSets[set] = descriptorSet;
}

void vulkan::CommandBuffer::rebind_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint)
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set];
	std::vector<uint32_t> dynamicOffsets{};

	Utility::for_each_bit(setLayout.uniformBuffer, [&](size_t binding) {
		for (size_t i = 0; i < setLayout.arraySizes[binding]; i++)
		{
			dynamicOffsets.push_back(static_cast<uint32_t>(m_resourceBindings.dynamicOffsets[set][binding][i]));
		}
		});
	Utility::for_each_bit(setLayout.storageBuffer, [&](size_t binding) {
		for (size_t i = 0; i < setLayout.arraySizes[binding]; i++)
		{
			dynamicOffsets.push_back(static_cast<uint32_t>(m_resourceBindings.dynamicOffsets[set][binding][i]));
		}
		});
	vkCmdBindDescriptorSets(m_vkHandle, bindPoint,
				m_currentPipelineLayout->get_layout(), set, 1, &m_allocatedDescriptorSets[set], static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
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
	vkCmdCopyBuffer(m_vkHandle, src.get_handle(), dst.get_handle(), copyCount, copies);
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
		vkCmdBlitImage(m_vkHandle, src.get_handle(), src.is_optimal() ? VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL, 
			dst.get_handle(), dst.is_optimal() ? VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
			1, &blit, filter);
	}
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
	vkCmdCopyBufferToImage(m_vkHandle, buffer.get_handle(), image.get_handle(),
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
	vkCmdCopyAccelerationStructureKHR(m_vkHandle, &info);
}

void vulkan::CommandBuffer::mip_barrier(const Image& image, VkImageLayout layout, VkPipelineStageFlags srcStage, VkAccessFlags srcAccess, bool needBarrier)
{
	assert(image.get_info().mipLevels > 1);
	std::array<VkImageMemoryBarrier, 2> barriers;
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
	assert(!m_currentRenderpass);
	assert(!m_currentFramebuffer);
	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	vkCmdPipelineBarrier(m_vkHandle, srcStages, dstStages, 0, barrierCount,
		globals, bufferBarrierCounts, bufferBarriers, imageBarrierCounts, imageBarriers);
}

void vulkan::CommandBuffer::barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess)
{
	assert(!m_currentRenderpass);
	assert(!m_currentFramebuffer);
	VkMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = srcAccess,
		.dstAccessMask = dstAccess
	};
	vkCmdPipelineBarrier(m_vkHandle, srcStages, dstStages, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void vulkan::CommandBuffer::image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags, VkPipelineStageFlags dstStages, VkAccessFlags dstAccessFlags)
{
	assert(!m_currentRenderpass);
	assert(!m_currentFramebuffer);

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
	vkCmdPipelineBarrier(m_vkHandle, srcStages, dstStages, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void vulkan::CommandBuffer::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	assert(!m_isCompute);
	if (flush_graphics()) {
		vkCmdDraw(m_vkHandle, vertexCount, instanceCount, firstVertex, firstInstance);
	}
	else {
		std::cout << "Could not flush render state, dropped draw call.\n";
	}
}

void vulkan::CommandBuffer::draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	assert(!m_isCompute);
	assert(m_indexState.buffer != VK_NULL_HANDLE);
	if (flush_graphics()) {
		vkCmdDrawIndexed(m_vkHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	else {
		std::cout << "Could not flush render state, dropped draw call.\n";
	}
}

void vulkan::CommandBuffer::dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
{
	if (flush_compute()) {
		vkCmdDispatch(m_vkHandle, groupsX, groupsY, groupsZ);
	}
	else {
		std::cout << "Could not flush compute state, dropped dispatch.\n";
	}
}

void vulkan::CommandBuffer::trace_rays(uint32_t width, uint32_t height, uint32_t depth)
{
	if (flush_ray()) {
		vkCmdTraceRaysKHR(m_vkHandle, nullptr, nullptr, nullptr, nullptr, width, height, depth);
	}
	else {
		std::cout << "Could not flush ray state, dropped ray trace call.\n";
	}
}

bool vulkan::CommandBuffer::swapchain_touched() const noexcept
{
	return m_swapchainTouched;
}

void vulkan::CommandBuffer::touch_swapchain() noexcept
{
	m_swapchainTouched = true;
}

void vulkan::CommandBuffer::bind_storage_image(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view)
{
	assert(view.get_image()->get_usage() & VK_IMAGE_USAGE_STORAGE_BIT);
	bind_texture(set, binding, arrayIndex, view.get_image_view(), view.get_image_view(), view.get_image()->get_info().layout, view.get_id());
}

void vulkan::CommandBuffer::bind_input_attachment(uint32_t set, uint32_t startBinding)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	uint32_t inputAttachmentCount = m_currentRenderpass->get_num_input_attachments(m_pipelineState.subpassIndex);

	if (m_resourceBindings.bindings[set][startBinding].size() < inputAttachmentCount) {
		m_resourceBindings.bindings[set][startBinding].resize(m_resourceBindings.bindings[set][startBinding].size() + inputAttachmentCount);
	}
	if (m_resourceBindings.bindingIds[set][startBinding].size() < inputAttachmentCount) {
		m_resourceBindings.bindingIds[set][startBinding].resize(m_resourceBindings.bindingIds[set][startBinding].size() + inputAttachmentCount);
	}
	for (size_t i = 0; i < inputAttachmentCount; i++ ){
		auto& inputAttachmentReference = m_currentRenderpass->get_input_attachment(static_cast<uint32_t>(i), m_pipelineState.subpassIndex);
		if (inputAttachmentReference.attachment == VK_ATTACHMENT_UNUSED)
			continue;

		const auto* view = m_framebufferAttachments[inputAttachmentReference.attachment];
		auto& image = m_resourceBindings.bindings[set][startBinding][i].image;
		assert(view);
		assert(view->get_image()->get_usage() & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
		if (view->get_id() == m_resourceBindings.bindingIds[set][startBinding][i] &&
			image.fp.imageLayout == inputAttachmentReference.layout)
			continue;

		image.fp.imageLayout = inputAttachmentReference.layout;
		image.fp.imageView = view->get_image_view();
		image.integer.imageLayout = inputAttachmentReference.layout;
		image.integer.imageView = view->get_image_view();
		m_resourceBindings.bindingIds[set][startBinding][i] = view->get_id();
		m_dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_acceleration_structure(uint32_t set, uint32_t binding, uint32_t arrayIndex, const AccelerationStructure& accelerationStructure)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);

	if (m_resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}

	if (accelerationStructure.get_id() == m_resourceBindings.bindingIds[set][binding][arrayIndex])
		return;
	m_dirtyDescriptorSets.set(set);
	m_resourceBindings.bindingIds[set][binding][arrayIndex] = accelerationStructure.get_id();
	m_resourceBindings.bindings[set][binding][arrayIndex].accelerationStructure;

}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view, const Sampler* sampler)
{
	bind_sampler(set, binding, arrayIndex, sampler);
	bind_texture(set, binding, arrayIndex, view.get_image_view(), view.get_image_view(),view.get_image()->get_info().layout, view.get_id());
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view, DefaultSampler sampler)
{
	assert(view.get_image()->get_usage() & VK_IMAGE_USAGE_SAMPLED_BIT);
	bind_texture(set, binding, arrayIndex, view, r_device.get_default_sampler(sampler));
}

void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Sampler* sampler)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);


	if (m_resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (sampler->get_id() == m_resourceBindings.samplerIds[set][binding][arrayIndex])
		return;
	auto& image = m_resourceBindings.bindings[set][binding][arrayIndex].image;
	image.fp.sampler = sampler->get_handle();
	image.integer.sampler = sampler->get_handle();
	m_dirtyDescriptorSets.set(set);
	m_resourceBindings.samplerIds[set][binding][arrayIndex] = sampler->get_id();
}

void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, DefaultSampler sampler)
{
	bind_sampler(set, binding, arrayIndex, r_device.get_default_sampler(sampler));
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);


	if (m_resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	auto& image = m_resourceBindings.bindings[set][binding][arrayIndex].image;
	auto& id = m_resourceBindings.bindingIds[set][binding][arrayIndex];
	if (id == bindingID && image.fp.imageLayout == layout)
		return;
	image.fp.imageLayout = layout;
	image.fp.imageView = floatView;
	image.integer.imageLayout = layout;
	image.integer.imageView = integerView;
	id = bindingID;
	m_dirtyDescriptorSets.set(set);

}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);
	if (m_resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	auto& bind = m_resourceBindings.bindings[set][binding][arrayIndex];
	if (buffer.get_id() == m_resourceBindings.bindingIds[set][binding][arrayIndex] && bind.buffer.range == size) {
		if (m_resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex ||
			m_resourceBindings.dynamicOffsets[set][binding][arrayIndex] != offset) {
			m_dirtyDescriptorSetsDynamicOffsets.set(set);
			m_resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
			m_resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		}
	}
	else {
		bind.buffer = VkDescriptorBufferInfo{
			.buffer = buffer.get_handle(),
			.offset = 0,
			.range = size
		};
		if (m_resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex) {
			m_resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
		}
		m_resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		m_resourceBindings.bindingIds[set][binding][arrayIndex] = buffer.get_id();
		m_resourceBindings.samplerIds[set][binding][arrayIndex] = 0;
		m_dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer)
{
	bind_uniform_buffer(set, binding, arrayIndex, buffer, 0, buffer.get_info().size);
}

void vulkan::CommandBuffer::bind_vertex_buffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkVertexInputRate inputRate)
{
	assert(binding < MAX_VERTEX_BINDINGS);
	assert(m_currentFramebuffer);

	
	if (buffer.get_handle() != m_vertexState.buffers[binding] || m_vertexState.offsets[binding] != offset)
		m_vertexState.dirty.set(binding);
	
	
	if (m_pipelineState.state.dynamic_vertex_input_binding_stride) {
		m_dynamicState.vertexStrides[binding] = inputRate;
	} 
	else {
		if (m_pipelineState.inputRates[binding] != inputRate)
			m_invalidFlags.set(InvalidFlags::StaticVertex);
		m_vertexState.buffers[binding] = buffer.get_handle();
		m_vertexState.offsets[binding] = offset;
		m_pipelineState.inputRates.set(binding, inputRate);
	}
}

VkCommandBuffer vulkan::CommandBuffer::get_handle() const noexcept
{
	return m_vkHandle;
}

void vulkan::CommandBuffer::end()
{
	if (auto result = vkEndCommandBuffer(m_vkHandle); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not end command buffer, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not end command buffer, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

void vulkan::CommandBuffer::begin_region(const char* name, const float* color)
{
	if constexpr (debug) {
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
			vkCmdBeginDebugUtilsLabelEXT(m_vkHandle, &label);
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
			vkCmdDebugMarkerBeginEXT(m_vkHandle, &marker);
		}
	}
}

void vulkan::CommandBuffer::end_region()
{
	if constexpr (debug) {
		if (r_device.get_supported_extensions().debug_utils) {
			vkCmdEndDebugUtilsLabelEXT(m_vkHandle);
		}
		else if (r_device.get_supported_extensions().debug_marker) {
			vkCmdDebugMarkerEndEXT(m_vkHandle);
		}
	}
}

vulkan::CommandBuffer::Type vulkan::CommandBuffer::get_type() const noexcept {
	return m_type;
}

void vulkan::CommandBuffer::set_depth_test(VkBool32 enabled) noexcept {
	if (m_pipelineState.state.dynamic_depth_test) {
		if (m_dynamicState.depth_test != static_cast<unsigned>(enabled)) {
			m_dynamicState.depth_test = static_cast<unsigned>(enabled);
			m_invalidFlags.set(InvalidFlags::DepthTest);
		}
	}
	else {
		if (m_pipelineState.state.depth_test != static_cast<unsigned>(enabled)) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.depth_test = static_cast<unsigned>(enabled);
		}
	}
}
void vulkan::CommandBuffer::set_depth_write(VkBool32 enabled) noexcept {
	if (m_pipelineState.state.dynamic_depth_write) {
		if (m_dynamicState.depth_write != static_cast<unsigned>(enabled)) {
			m_dynamicState.depth_write = static_cast<unsigned>(enabled);
			m_invalidFlags.set(InvalidFlags::DepthWrite);
		}
	}
	else {
		if (m_pipelineState.state.depth_write != enabled) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.depth_write = enabled;
		}
	}
}
void vulkan::CommandBuffer::set_depth_compare(VkCompareOp compare) noexcept {
	if (m_pipelineState.state.dynamic_depth_compare) {
		if (m_dynamicState.depth_compare != static_cast<unsigned>(compare)) {
			m_dynamicState.depth_compare = static_cast<unsigned>(compare);
			m_invalidFlags.set(InvalidFlags::DepthCompare);
		}
	}
	else {
		if (m_pipelineState.state.depth_compare != static_cast<unsigned>(compare)) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.depth_compare = static_cast<unsigned>(compare);
		}
	}
}
void vulkan::CommandBuffer::set_cull_mode(VkCullModeFlags cullMode) noexcept {
	if (m_pipelineState.state.dynamic_cull_mode) {
		if (m_dynamicState.cull_mode != static_cast<unsigned>(cullMode)) {
			m_dynamicState.cull_mode = static_cast<unsigned>(cullMode);
			m_invalidFlags.set(InvalidFlags::CullMode);
		}
	}
	else {
		if (m_pipelineState.state.cull_mode != static_cast<unsigned>(cullMode)) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.cull_mode = static_cast<unsigned>(cullMode);
		}
	}
}
void vulkan::CommandBuffer::set_front_face(VkFrontFace frontFace) noexcept {
	if (m_pipelineState.state.dynamic_front_face) {
		if (m_dynamicState.front_face != static_cast<unsigned>(frontFace)) {
			m_dynamicState.front_face = static_cast<unsigned>(frontFace);
			m_invalidFlags.set(InvalidFlags::FrontFace);
		}
	}
	else {
		if (m_pipelineState.state.front_face != static_cast<unsigned>(frontFace)) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.front_face = static_cast<unsigned>(frontFace);
		}
	}
}
void vulkan::CommandBuffer::set_topology(VkPrimitiveTopology topology) noexcept {
	if (m_pipelineState.state.dynamic_primitive_topology) {
		if (m_dynamicState.topology != static_cast<unsigned>(topology)) {
			m_dynamicState.topology = static_cast<unsigned>(topology);
			m_invalidFlags.set(InvalidFlags::FrontFace);
		}
	}
	else {
		if (m_pipelineState.state.topology != static_cast<unsigned>(topology)) {
			m_invalidFlags.set(InvalidFlags::StaticPipeline);
			m_pipelineState.state.topology = static_cast<unsigned>(topology);
		}
	}
}
void vulkan::CommandBuffer::set_blend_enable(VkBool32 enabled) noexcept {
	if (m_pipelineState.state.blend_enable != static_cast<unsigned>(enabled)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.blend_enable = static_cast<unsigned>(enabled);
	}
}
void vulkan::CommandBuffer::set_src_color_blend(VkBlendFactor blendFactor) noexcept {
	if (m_pipelineState.state.src_color_blend != static_cast<unsigned>(blendFactor)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.src_color_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_dst_color_blend(VkBlendFactor blendFactor) noexcept {
	if (m_pipelineState.state.dst_color_blend != static_cast<unsigned>(blendFactor)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.dst_color_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_src_alpha_blend(VkBlendFactor blendFactor) noexcept {
	if (m_pipelineState.state.src_alpha_blend != static_cast<unsigned>(blendFactor)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.src_alpha_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_dst_alpha_blend(VkBlendFactor blendFactor) noexcept {
	if (m_pipelineState.state.dst_alpha_blend != static_cast<unsigned>(blendFactor)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.dst_alpha_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::reset_pipeline_state() noexcept {
	if (m_pipelineState.state != defaultPipelineState) {
		m_pipelineState.state = defaultPipelineState;
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
	}
}
void vulkan::CommandBuffer::set_polygon_mode(VkPolygonMode polygon_mode) noexcept {
	if (m_pipelineState.state.polygon_mode != static_cast<unsigned>(polygon_mode)) {
		m_invalidFlags.set(InvalidFlags::StaticPipeline);
		m_pipelineState.state.polygon_mode = static_cast<unsigned>(polygon_mode);
	}
}
void vulkan::CommandBuffer::disable_depth() noexcept {
	set_depth_test(VK_FALSE);
	set_depth_write(VK_FALSE);
}
void vulkan::CommandBuffer::enable_alpha() noexcept {
	set_blend_enable(VK_TRUE);
	set_src_color_blend(VK_BLEND_FACTOR_SRC_ALPHA);
	set_dst_color_blend(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
	set_src_alpha_blend(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
	set_dst_alpha_blend(VK_BLEND_FACTOR_ZERO);
}

void vulkan::CommandBuffer::bind_vertex_buffers() noexcept
{
	auto updateMask = m_vertexState.active & m_vertexState.dirty;
	Utility::for_each_bitrange(updateMask, [&](uint32_t binding, uint32_t range) {
		for (uint32_t i = binding; i < binding + range; i++)
			assert(m_vertexState.buffers[i] != VK_NULL_HANDLE);
		auto [buffers, offsets] = m_vertexState[binding];
		if (m_pipelineState.state.dynamic_vertex_input_binding_stride)
			vkCmdBindVertexBuffers2EXT(m_vkHandle, binding, range, buffers, offsets, nullptr, m_dynamicState.vertexStrides.data() + binding);
		else
			vkCmdBindVertexBuffers(m_vkHandle,binding, range, buffers, offsets);
	});
	m_vertexState.update(updateMask);
}

void vulkan::CommandBuffer::push_constants(const void* data, VkDeviceSize offset, VkDeviceSize range)
{
	//TODO constant
	assert(offset + range <= 128);
	memcpy(m_resourceBindings.pushConstantData.data() + offset, data, range);
	m_invalidFlags.set(InvalidFlags::PushConstants);
}

void vulkan::CommandBuffer::bind_index_buffer(IndexState indexState)
{
	if (m_indexState == indexState)
		return;
	m_indexState = indexState;
	vkCmdBindIndexBuffer(m_vkHandle, indexState.buffer, indexState.offset, indexState.indexType);
}

void vulkan::CommandBuffer::init_viewport_and_scissor(const RenderpassCreateInfo& info)
{
	assert(m_currentFramebuffer);
	m_scissor = info.renderArea;
	auto [width, height] = m_currentFramebuffer->get_extent();
	m_scissor.offset.x = Math::min(m_scissor.offset.x, static_cast<int32_t>(width));
	m_scissor.offset.y = Math::min(m_scissor.offset.y, static_cast<int32_t>(height));
	m_scissor.extent.width = Math::min(width - m_scissor.offset.x, m_scissor.extent.width);
	m_scissor.extent.height = Math::min(height - m_scissor.offset.y, m_scissor.extent.height);

	m_viewport = VkViewport {
		.x = 0.0f,
		.y = 0.0f,
		.width = static_cast<float>(width),
		.height = static_cast<float>(height),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
}

void vulkan::CommandBuffer::begin_render_pass(const RenderpassCreateInfo& renderpassInfo, VkSubpassContents contents)
{
	m_currentRenderpass = r_device.request_render_pass(renderpassInfo);
	m_currentFramebuffer = r_device.request_framebuffer(renderpassInfo);
	m_pipelineState.subpassIndex = 0;
	m_pipelineState.compatibleRenderPass = r_device.request_compatible_render_pass(renderpassInfo);
	
	init_viewport_and_scissor(renderpassInfo);
	m_framebufferAttachments = {};
	std::array<VkClearValue, MAX_ATTACHMENTS + 1> clearValues;
	uint32_t clearValueCount = static_cast<uint32_t>(renderpassInfo.colorAttachmentsCount);
	for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
		m_framebufferAttachments[i] = renderpassInfo.colorAttachmentsViews[i];
		if (renderpassInfo.clearAttachments.test(i)) {
			clearValues[i].color = renderpassInfo.clearColors[i];
		}
		if (renderpassInfo.colorAttachmentsViews[i]->get_image()->get_info().isSwapchainImage)
			m_swapchainTouched = true;
	}
	m_framebufferAttachments[renderpassInfo.colorAttachmentsCount] = renderpassInfo.depthStencilAttachment;
	if (renderpassInfo.depthStencilAttachment && renderpassInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilClear)) {
		clearValues[clearValueCount++].depthStencil = renderpassInfo.clearDepthStencil;
		
	}
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_currentRenderpass->get_render_pass(),
		.framebuffer = m_currentFramebuffer->get_handle(),
		.renderArea = m_scissor,
		.clearValueCount = clearValueCount,
		.pClearValues = clearValues.data(),
	};

	vkCmdBeginRenderPass(m_vkHandle, &renderPassBeginInfo, contents);
	m_currentContents = contents;
	begin_graphics();
}

void vulkan::CommandBuffer::end_render_pass()
{
	assert(m_currentFramebuffer);// No Framebuffer, forgot to begin render pass?
	assert(m_currentRenderpass);
	assert(m_pipelineState.compatibleRenderPass);
	vkCmdEndRenderPass(m_vkHandle);
	m_currentFramebuffer = nullptr;
	m_currentRenderpass = nullptr;
	m_pipelineState.compatibleRenderPass = nullptr;
	begin_compute();
}

bool operator==(const VkPushConstantRange& lhs, const VkPushConstantRange& rhs) {
	return lhs.offset == rhs.offset && lhs.size == rhs.size && lhs.stageFlags == rhs.stageFlags;
}
bool operator!=(const VkPushConstantRange& lhs, const VkPushConstantRange& rhs) {
	return !(lhs == rhs);
}
void vulkan::CommandBuffer::set_vertex_attribute(uint32_t location, uint32_t binding, VkFormat format)
{
	assert(location < MAX_VERTEX_ATTRIBUTES);
	assert(m_currentFramebuffer);
	assert(binding < MAX_VERTEX_BINDINGS);
	m_vertexState.active.set(binding);
	auto [format_, binding_] = m_pipelineState.attributes[location];
	if (format_ == format && binding_ == binding)
		return;
	m_invalidFlags.set(InvalidFlags::StaticVertex);
	m_pipelineState.attributes.bindings[location] = static_cast<uint8_t>(binding);
	m_pipelineState.attributes.formats[location] = format;

}
void vulkan::CommandBuffer::set_scissor(VkRect2D scissor)
{
	m_invalidFlags.set(InvalidFlags::Scissor);
	m_scissor = scissor;
}
VkRect2D vulkan::CommandBuffer::get_scissor() const
{
	return m_scissor;
}
void vulkan::CommandBuffer::bind_program(Program* program)
{
	if (!program)
		return;
	if (m_pipelineState.program == program)
		return;
	m_pipelineState.program = program;
	m_currentPipeline = VK_NULL_HANDLE;

	assert((m_currentFramebuffer && program->get_shader(ShaderStage::Vertex)) ||
		   (!m_currentFramebuffer && program->get_shader(ShaderStage::Compute)));
	assert(program->get_pipeline_layout() != nullptr);
	m_invalidFlags.set(InvalidFlags::Pipeline);
	m_invalidFlags.set(InvalidFlags::DynamicState);
	if (!m_currentPipelineLayout) {
		m_dirtyDescriptorSets.set();
		m_invalidFlags.set(InvalidFlags::PushConstants);
		m_currentPipelineLayout = program->get_pipeline_layout();
	}
	else if (auto* new_layout = program->get_pipeline_layout(); new_layout->get_hash() != m_currentPipelineLayout->get_hash()) {
		auto& newShaderLayout = new_layout->get_shader_layout();
		auto& oldShaderLayout = m_currentPipelineLayout->get_shader_layout();
		if (newShaderLayout.pushConstantRange != oldShaderLayout.pushConstantRange) {
			m_dirtyDescriptorSets.set();
			m_invalidFlags.set(InvalidFlags::PushConstants);
		}
		else {
			for (uint32_t i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
				if (new_layout->get_allocator(i) != m_currentPipelineLayout->get_allocator(i)) {
					decltype(m_dirtyDescriptorSets) update{};
					for (uint32_t j = 0; j < i; j++) {
						update.set(j);
					}
					m_dirtyDescriptorSets &= ~update;
					break;
				}
			}
		}
		m_currentPipelineLayout = new_layout;
	}
}
