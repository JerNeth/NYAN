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
	flush_descriptor_sets();

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
	flush_descriptor_sets();

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

void vulkan::CommandBuffer::flush_descriptor_sets()
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	
	auto setUpdate = shaderLayout.used & m_dirtyDescriptorSets;
	Utility::for_each_bit(setUpdate, [&](uint32_t set) {
		flush_descriptor_set(set);
	});
	m_dirtyDescriptorSets &= ~setUpdate;

	m_dirtyDescriptorSetsDynamicOffsets &= ~setUpdate;

	auto dynamicOffsetUpdates = shaderLayout.used & m_dirtyDescriptorSetsDynamicOffsets;
	Utility::for_each_bit(dynamicOffsetUpdates, [&](uint32_t set) {
		rebind_descriptor_set(set);
	});
	m_dirtyDescriptorSetsDynamicOffsets &= ~dynamicOffsetUpdates;

}

void vulkan::CommandBuffer::flush_descriptor_set(uint32_t set)
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set];
	uint32_t dynamicOffsetCount = 0;
	std::array<uint32_t, MAX_BINDINGS> dynamicOffsets{};
	Utility::Hasher hasher;

	hasher(setLayout.fp);

	Utility::for_each_bit(setLayout.uniformBuffer, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			hasher(m_resourceBindings.bindings[set][binding + i].buffer.range);
			assert(m_resourceBindings.bindings[set][binding + i].buffer.buffer != VK_NULL_HANDLE);
			assert(dynamicOffsetCount < MAX_BINDINGS);
			dynamicOffsets[dynamicOffsetCount++] = m_resourceBindings.bindings[set][binding + i].dynamicOffset;
		}
	});

	Utility::for_each_bit(setLayout.storageBuffer, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			hasher(m_resourceBindings.bindings[set][binding + i].buffer.range);
			hasher(m_resourceBindings.bindings[set][binding + i].buffer.offset);
			assert(m_resourceBindings.bindings[set][binding + i].buffer.buffer != VK_NULL_HANDLE);
		}
	});

	Utility::for_each_bit(setLayout.sampledBuffer, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			assert(m_resourceBindings.bindings[set][binding + i].bufferView != VK_NULL_HANDLE);
		}
	});

	Utility::for_each_bit(setLayout.imageSampler, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			if (!setLayout.immutableSampler.test(binding + i)) {
				hasher(m_resourceBindings.samplerIds[set][binding + i]);
				assert(m_resourceBindings.bindings[set][binding + i].image.fp.sampler != VK_NULL_HANDLE);
			}
			hasher(m_resourceBindings.bindings[set][binding + i].image.fp.imageLayout);
			assert(m_resourceBindings.bindings[set][binding + i].image.fp.imageView != VK_NULL_HANDLE);
		}
	});
	Utility::for_each_bit(setLayout.separateImage, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			hasher(m_resourceBindings.bindings[set][binding + i].image.fp.imageLayout);
			assert(m_resourceBindings.bindings[set][binding + i].image.fp.imageView != VK_NULL_HANDLE);
		}
	});
	Utility::for_each_bit(setLayout.seperateSampler, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.samplerIds[set][binding + i]);
			assert(m_resourceBindings.bindings[set][binding + i].image.fp.sampler != VK_NULL_HANDLE);
		}
	});

	Utility::for_each_bit(setLayout.storageImage, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			hasher(m_resourceBindings.bindings[set][binding + i].image.fp.imageLayout);
			assert(m_resourceBindings.bindings[set][binding + i].image.fp.imageView != VK_NULL_HANDLE);
		}
	});
	Utility::for_each_bit(setLayout.inputAttachment, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) {
			hasher(m_resourceBindings.bindingIds[set][binding + i]);
			hasher(m_resourceBindings.bindings[set][binding + i].image.fp.imageLayout);
			assert(m_resourceBindings.bindings[set][binding + i].image.fp.imageView != VK_NULL_HANDLE);
		}
	});
	auto hash = hasher();

	VkDescriptorSet descriptorSet;
	bool allocated;
	std::tie(descriptorSet, allocated) =m_currentPipelineLayout->get_allocator(set)->find(m_threadIdx, hash);
	if (!allocated) {
		auto updateTemplate = m_currentPipelineLayout->get_update_template(set);

		if (updateTemplate != VK_NULL_HANDLE) {
			vkUpdateDescriptorSetWithTemplate(r_device.get_device(), descriptorSet, updateTemplate, m_resourceBindings.bindings[set].data());
		}

	}
	vkCmdBindDescriptorSets(m_vkHandle, m_isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_currentPipelineLayout->get_layout(), set, 1, &descriptorSet, dynamicOffsetCount, dynamicOffsets.data() );
	m_allocatedDescriptorSets[set] = descriptorSet;
}

void vulkan::CommandBuffer::rebind_descriptor_set(uint32_t set)
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set];
	uint32_t dynamicOffsetCount = 0;
	std::array<uint32_t, MAX_BINDINGS> dynamicOffsets;

	Utility::for_each_bit(setLayout.uniformBuffer, [&](uint32_t binding) {
		uint32_t arraySize = setLayout.arraySizes[binding];
		for (uint32_t i = 0; i < arraySize; i++) 
		{
			assert(dynamicOffsetCount < MAX_BINDINGS);
			dynamicOffsets[dynamicOffsetCount++] = m_resourceBindings.bindings[set][binding + i].dynamicOffset;
		}
	});
	vkCmdBindDescriptorSets(m_vkHandle, m_isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
				m_currentPipelineLayout->get_layout(), set, 1, &m_allocatedDescriptorSets[set], dynamicOffsetCount, dynamicOffsets.data());
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
	assert(m_isCompute);
	if (flush_compute()) {
		vkCmdDispatch(m_vkHandle, groupsX, groupsY, groupsZ);
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

void vulkan::CommandBuffer::bind_storage_image(uint32_t set, uint32_t binding, const ImageView& view)
{
	assert(view.get_image()->get_usage() & VK_IMAGE_USAGE_STORAGE_BIT);
	bind_texture(set, binding, view.get_image_view(), view.get_image_view(), view.get_image()->get_info().layout, view.get_id());
}

void vulkan::CommandBuffer::bind_input_attachment(uint32_t set, uint32_t startBinding)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	uint32_t inputAttachmentCount = m_currentRenderpass->get_num_color_attachments(m_pipelineState.subpassIndex);
	assert(startBinding + inputAttachmentCount <= MAX_BINDINGS);
	for (uint32_t i = 0; i < inputAttachmentCount; i++ ){
		auto& inputAttachmentReference = m_currentRenderpass->get_input_attachment(i, m_pipelineState.subpassIndex);
		if (inputAttachmentReference.attachment == VK_ATTACHMENT_UNUSED)
			continue;

		const auto* view = m_framebufferAttachments[inputAttachmentReference.attachment];
		auto& image = m_resourceBindings.bindings[set][startBinding + i].image;
		assert(view);
		assert(view->get_image()->get_usage() & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
		if (view->get_id() == m_resourceBindings.bindingIds[set][startBinding + i] &&
			image.fp.imageLayout == inputAttachmentReference.layout)
			continue;

		image.fp.imageLayout = inputAttachmentReference.layout;
		image.fp.imageView = view->get_image_view();
		image.integer.imageLayout = inputAttachmentReference.layout;
		image.integer.imageView = view->get_image_view();
		m_resourceBindings.bindingIds[set][startBinding + i] = view->get_id();
		m_dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, const ImageView& view, const Sampler* sampler)
{
	bind_sampler(set, binding, sampler);
	bind_texture(set, binding, view.get_image_view(), view.get_image_view(),view.get_image()->get_info().layout, view.get_id());
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, const ImageView& view, DefaultSampler sampler)
{
	assert(view.get_image()->get_usage() & VK_IMAGE_USAGE_SAMPLED_BIT);
	bind_texture(set, binding, view, r_device.get_default_sampler(sampler));
}

void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, const Sampler* sampler)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);
	if (sampler->get_id() == m_resourceBindings.samplerIds[set][binding])
		return;
	auto& image = m_resourceBindings.bindings[set][binding].image;
	image.fp.sampler = sampler->get_handle();
	image.integer.sampler = sampler->get_handle();
	m_dirtyDescriptorSets.set(set);
	m_resourceBindings.samplerIds[set][binding] = sampler->get_id();
}

void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, DefaultSampler sampler)
{
	bind_sampler(set, binding, r_device.get_default_sampler(sampler));
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);


	auto& image = m_resourceBindings.bindings[set][binding].image;
	auto& id = m_resourceBindings.bindingIds[set][binding];
	if (id == bindingID && image.fp.imageLayout == layout)
		return;
	image.fp.imageLayout = layout;
	image.fp.imageView = floatView;
	image.integer.imageLayout = layout;
	image.integer.imageView = integerView;
	id = bindingID;
	m_dirtyDescriptorSets.set(set);

}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);
	auto& bind = m_resourceBindings.bindings[set][binding];
	if (buffer.get_id() == m_resourceBindings.bindingIds[set][binding] && bind.buffer.range == size) {
		if (bind.dynamicOffset != offset) {
			m_dirtyDescriptorSetsDynamicOffsets.set(set);
			bind.dynamicOffset = offset;
		}
	}
	else {
		bind.buffer = VkDescriptorBufferInfo{
			.buffer = buffer.get_handle(),
			.offset = 0,
			.range = size
		};
		bind.dynamicOffset = offset;
		m_resourceBindings.bindingIds[set][binding] = buffer.get_id();
		m_resourceBindings.samplerIds[set][binding] = 0;
		m_dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, const Buffer& buffer)
{
	bind_uniform_buffer(set, binding, buffer, 0, buffer.get_info().size);
}

void vulkan::CommandBuffer::bind_vertex_buffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkVertexInputRate inputRate, VkDeviceSize vertexStride)
{
	assert(binding < MAX_VERTEX_BINDINGS);
	assert(m_currentFramebuffer);

	
	if (buffer.get_handle() != m_vertexState.buffers[binding] || m_vertexState.offsets[binding] != offset)
		m_vertexState.dirty.set(binding);
	
	
	if (m_pipelineState.state.dynamic_vertex_input_binding_stride)
		m_dynamicState.vertexStrides[binding] = inputRate;
	if (m_pipelineState.inputRates[binding] != inputRate)
		m_invalidFlags.set(InvalidFlags::StaticVertex);
	m_vertexState.buffers[binding] = buffer.get_handle();
	m_vertexState.offsets[binding] = offset;
	m_pipelineState.inputRates.set(binding, inputRate);
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
	std::memcpy(m_resourceBindings.pushConstantData.data() + offset, data, range);
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
	m_swapchainTouched = true;
	m_currentContents = contents;
	begin_graphics();
}

void vulkan::CommandBuffer::end_render_pass()
{
	assert(m_currentFramebuffer, "No Framebuffer, forgot to begin render pass?");
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
	m_pipelineState.attributes.bindings[location] = binding;
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
