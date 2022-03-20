#include "CommandBuffer.h"
#include "LogicalDevice.h"
vulkan::CommandBuffer::CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type, uint32_t threadIdx, bool tiny) :
	r_device(parent),
	m_vkHandle(handle),
	m_threadIdx(threadIdx),
	m_type(type),
	m_tiny(tiny)
{
	assert(m_vkHandle);
	if (!tiny) {
		m_data = std::make_unique<Data>();
		if (type == Type::Generic) {
			m_data->pipelineState.state = defaultPipelineState;
			//m_pipelineState.state.dynamic_vertex_input = 1;
			//m_pipelineState.state.dynamic_vertex_input_binding_stride = 1;
			m_data->pipelineState.state.dynamic_depth_test = 1;
			m_data->pipelineState.state.dynamic_depth_write = 1;
			m_data->pipelineState.state.dynamic_cull_mode = 1;
			m_data->dynamicState.cull_mode = defaultPipelineState.cull_mode;
			m_data->dynamicState.depth_test = defaultPipelineState.depth_test;
			m_data->dynamicState.depth_write = defaultPipelineState.depth_write;
		}
		m_data->pipelineState.program = nullptr;
		m_data->resourceBindings.bindings = {};
	}
	if constexpr (debug) {
#ifdef DEBUGSUBMISSIONS
		std::cout << "Created commandbuffer with handle: " << handle << '\n';
#endif
	}
}

void vulkan::CommandBuffer::begin_rendering(const VkRenderingInfo* info)
{
	vkCmdBeginRendering(m_vkHandle, info);
}

void vulkan::CommandBuffer::end_rendering()
{
	vkCmdEndRendering(m_vkHandle);
}

vulkan::GraphicsPipelineBind vulkan::CommandBuffer::bind_graphics_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_pipeline());
	const auto& dynamicState = pipeline->get_dynamic_state();
	//vkCmdSetRasterizerDiscardEnable(m_vkHandle, dynamicState.rasterizer_discard_enable);
	//vkCmdSetDepthBiasEnable(m_vkHandle, dynamicState.depth_bias_enable);
	//vkCmdSetPrimitiveRestartEnable(m_vkHandle, dynamicState.primitive_restart_enable);
	//vkCmdSetPrimitiveTopology(m_vkHandle, dynamicState.primitive_topology);
	//auto set = r_device.get_bindless_set().get_set();
	//vkCmdBindDescriptorSets(m_vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->get_layout(), 0, 1, &set, 0, nullptr);
	return GraphicsPipelineBind(m_vkHandle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_GRAPHICS);
}

vulkan::ComputePipelineBind vulkan::CommandBuffer::bind_compute_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->get_pipeline());
	auto set = r_device.get_bindless_set().get_set();
	vkCmdBindDescriptorSets(m_vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline->get_layout(), 0, 1, &set, 0, nullptr);
	return ComputePipelineBind(m_vkHandle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_COMPUTE);
}

vulkan::RaytracingPipelineBind vulkan::CommandBuffer::bind_raytracing_pipeline(PipelineId pipelineIdentifier)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(pipelineIdentifier);
	vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->get_pipeline());
	auto set = r_device.get_bindless_set().get_set();
	vkCmdBindDescriptorSets(m_vkHandle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline->get_layout(), 0, 1, &set, 0, nullptr);
	return RaytracingPipelineBind(m_vkHandle, pipeline->get_layout(), VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR);
}



void vulkan::CommandBuffer::begin_context()
{
	assert(!m_tiny);
	m_data->invalidFlags.set();
	m_data->indexState = {};
	m_data->vertexState.buffers = {};
	m_data->vertexState.dirty.set();
	m_data->dirtyDescriptorSets.set();
	m_data->bindlessDescriptorSets.reset();
	m_data->currentPipeline = VK_NULL_HANDLE;
	m_data->currentPipelineLayout = nullptr;
	m_data->pipelineState.program = nullptr;

	
	m_data->resourceBindings.bindingIds = {};
	m_data->resourceBindings.samplerIds = {};
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
	assert(!m_tiny);
	OPTICK_GPU_EVENT("Flush Graphics");
	if (!m_data->pipelineState.program)
		return false;
	assert(m_data->currentPipelineLayout != nullptr);

	if (m_data->currentPipeline == VK_NULL_HANDLE)
		m_data->invalidFlags.set(InvalidFlags::Pipeline);

	if (m_data->invalidFlags.get_and_clear(InvalidFlags::StaticPipeline, InvalidFlags::Pipeline, InvalidFlags::StaticVertex)) {
		VkPipeline oldPipeline = m_data->currentPipeline;
		if (!flush_graphics_pipeline())
			return false;
		if (oldPipeline != m_data->currentPipeline) {
			vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_GRAPHICS, m_data->currentPipeline);
			invalidate_dynamic_state();
		}
	}

	if (m_data->currentPipeline == VK_NULL_HANDLE)
		return false;

	update_dynamic_vertex_state();

	flush_descriptor_sets(VK_PIPELINE_BIND_POINT_GRAPHICS);

	update_push_constants();
	update_dynamic_state();
	bind_vertex_buffers();
	return true;
}

bool vulkan::CommandBuffer::flush_compute()
{
	assert(!m_tiny);
	OPTICK_GPU_EVENT("Flush Compute");
	if (!m_data->pipelineState.program)
		return false;
	assert(m_data->currentPipelineLayout != nullptr);

	if (m_data->currentPipeline == VK_NULL_HANDLE)
		m_data->invalidFlags.set(InvalidFlags::Pipeline);

	if (m_data->invalidFlags.get_and_clear(InvalidFlags::Pipeline)) {
		VkPipeline oldPipeline = m_data->currentPipeline;
		if (!flush_compute_pipeline())
			return false;
		if (oldPipeline != m_data->currentPipeline) {
			vkCmdBindPipeline(m_vkHandle, VK_PIPELINE_BIND_POINT_COMPUTE, m_data->currentPipeline);
		}
	}
	if (m_data->currentPipeline == VK_NULL_HANDLE)
		return false;
	flush_descriptor_sets(VK_PIPELINE_BIND_POINT_COMPUTE);

	if (m_data->invalidFlags.get_and_clear(InvalidFlags::PushConstants)) {
		auto& pushConstants = m_data->currentPipelineLayout->get_shader_layout().pushConstantRange;

		if (pushConstants.stageFlags) {
			assert(pushConstants.offset == 0);
			//assert(false);
			vkCmdPushConstants(m_vkHandle, m_data->currentPipelineLayout->get_layout(), pushConstants.stageFlags, 0, pushConstants.size, m_data->resourceBindings.pushConstantData.data());
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
	assert(!m_tiny);
	m_data->currentPipeline = r_device.request_pipeline(m_data->pipelineState);
	return m_data->currentPipeline != VK_NULL_HANDLE;
}

bool vulkan::CommandBuffer::flush_compute_pipeline()
{
	assert(!m_tiny);
	assert(m_data->pipelineState.program);
	m_data->currentPipeline = r_device.request_pipeline(*m_data->pipelineState.program);
	return m_data->currentPipeline != VK_NULL_HANDLE;
}

bool vulkan::CommandBuffer::flush_ray_pipeline()
{
	return false;
}

void vulkan::CommandBuffer::flush_descriptor_sets(VkPipelineBindPoint bindPoint)
{
	assert(!m_tiny);
	auto& shaderLayout = m_data->currentPipelineLayout->get_shader_layout();
	
	auto setUpdate = shaderLayout.used & m_data->dirtyDescriptorSets & ~m_data->bindlessDescriptorSets;
	Utility::for_each_bit(setUpdate, [&](size_t set) {
		flush_descriptor_set(static_cast<uint32_t>(set), bindPoint);
	});
	m_data->dirtyDescriptorSets &= ~setUpdate;

	m_data->dirtyDescriptorSetsDynamicOffsets &= ~setUpdate;

	setUpdate = shaderLayout.used & m_data->dirtyDescriptorSets & m_data->bindlessDescriptorSets;

	Utility::for_each_bitrange(setUpdate, [&](size_t firstSet, size_t setCount) {
		flush_bindless_descriptor_sets(static_cast<uint32_t>(firstSet), setCount, bindPoint);
	});

	auto dynamicOffsetUpdates = shaderLayout.used & m_data->dirtyDescriptorSetsDynamicOffsets & ~m_data->bindlessDescriptorSets;
	Utility::for_each_bit(dynamicOffsetUpdates, [&](size_t set) {
		rebind_descriptor_set(static_cast<uint32_t>(set), bindPoint);
	});
	m_data->dirtyDescriptorSetsDynamicOffsets &= ~dynamicOffsetUpdates;

}

void vulkan::CommandBuffer::flush_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint)
{
	assert(!m_tiny);
	auto& shaderLayout = m_data->currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set].descriptors;
	std::vector<uint32_t> dynamicOffsets{};
	Utility::Hasher hasher;

	size_t bindingSize = 0;
	for (size_t binding = 0; binding < setLayout.size(); ++binding) {
		const auto& descriptorLayout = setLayout[binding];
		if(descriptorLayout.type == DescriptorType::Invalid)
			continue;
		bindingSize += descriptorLayout.arraySize;

		switch (descriptorLayout.type) {
			using enum DescriptorType;
			case Sampler:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.samplerIds[set][binding][array]);
					assert(m_data->resourceBindings.bindings[set][binding][array].image.sampler != VK_NULL_HANDLE);
				}
				break;
			case CombinedImageSampler:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					hasher(m_data->resourceBindings.bindings[set][binding][array].image.imageLayout);
					assert(m_data->resourceBindings.bindings[set][binding][array].image.imageView != VK_NULL_HANDLE);

					hasher(m_data->resourceBindings.samplerIds[set][binding][array]);
					assert(m_data->resourceBindings.bindings[set][binding][array].image.sampler != VK_NULL_HANDLE);
				}
				break;
			case SampledImage:
			case StorageImage:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					hasher(m_data->resourceBindings.bindings[set][binding][array].image.imageLayout);
					assert(m_data->resourceBindings.bindings[set][binding][array].image.imageView != VK_NULL_HANDLE);
				}
				break;
			case StorageTexelBuffer:
			case UniformTexelBuffer:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					assert(m_data->resourceBindings.bindings[set][binding][array].bufferView != VK_NULL_HANDLE);
				}
				break;
			case UniformBuffer:
			case StorageBuffer:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					hasher(m_data->resourceBindings.bindings[set][binding][array].buffer.offset);
					hasher(m_data->resourceBindings.bindings[set][binding][array].buffer.range);
					assert(m_data->resourceBindings.bindings[set][binding][array].buffer.buffer != VK_NULL_HANDLE);
				}
				break;
			case UniformDynamicBuffer:
			case StorageBufferDynamic:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					hasher(m_data->resourceBindings.bindings[set][binding][array].buffer.range);
					assert(m_data->resourceBindings.bindings[set][binding][array].buffer.buffer != VK_NULL_HANDLE);
					dynamicOffsets.push_back(static_cast<uint32_t>(m_data->resourceBindings.dynamicOffsets[set][binding][array]));
				}
				break;
			case InputAttachment:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					hasher(m_data->resourceBindings.bindings[set][binding][array].image.imageLayout);
					assert(m_data->resourceBindings.bindings[set][binding][array].image.imageView != VK_NULL_HANDLE);
				}
				break;
			case AccelerationStructure:
				for (size_t array = 0; array < descriptorLayout.arraySize; array++) {
					hasher(m_data->resourceBindings.bindingIds[set][binding][array]);
					assert(m_data->resourceBindings.bindings[set][binding][array].accelerationStructure != VK_NULL_HANDLE);
				}
				break;
			default:
				assert(false);
				break;

		}
	}
	auto hash = hasher();

	auto [descriptorSet, allocated] = m_data->currentPipelineLayout->get_allocator(set)->find(m_threadIdx, hash);
	if (!allocated) {
		auto updateTemplate = m_data->currentPipelineLayout->get_update_template(set);

		if (updateTemplate != VK_NULL_HANDLE) {
			//TODO redo
			m_data->bindingBuffer.clear();
			m_data->bindingBuffer.resize(bindingSize);
			size_t count = 0;
			for (size_t binding = 0; binding < MAX_BINDINGS; ++binding) {
				assert(m_data->resourceBindings.bindings[set][binding].size() == setLayout[binding].arraySize && "More Elements in shader array than bound");
				for (size_t array = 0; array < setLayout[binding].arraySize; array++) {
					m_data->bindingBuffer[count++] = m_data->resourceBindings.bindings[set][binding][array];
				}
			}
			vkUpdateDescriptorSetWithTemplate(r_device.get_device(), descriptorSet, updateTemplate, m_data->bindingBuffer.data());
			//vkCmdPushDescriptorSetWithTemplateKHR(m_vkHandle, updateTemplate, m_currentPipelineLayout->get_layout(),set, m_resourceBindings.bindings[set].data());
		}
		//Fallback maybe?

	}
	vkCmdBindDescriptorSets(m_vkHandle, bindPoint,
		m_data->currentPipelineLayout->get_layout(), set, 1, &descriptorSet, static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data() );
	m_data->allocatedDescriptorSets[set] = descriptorSet;
}

void vulkan::CommandBuffer::flush_bindless_descriptor_sets(uint32_t firstSet, uint32_t setCount, VkPipelineBindPoint bindPoint)
{
	vkCmdBindDescriptorSets(m_vkHandle, bindPoint,
		m_data->currentPipelineLayout->get_layout(), firstSet, setCount, &m_data->allocatedDescriptorSets[firstSet], 0, nullptr);
}

void vulkan::CommandBuffer::rebind_descriptor_set(uint32_t set, VkPipelineBindPoint bindPoint)
{
	assert(!m_tiny);
	auto& shaderLayout = m_data->currentPipelineLayout->get_shader_layout();
	auto& setLayout = shaderLayout.descriptors[set].descriptors;
	std::vector<uint32_t> dynamicOffsets{};


	for (size_t binding = 0; binding < setLayout.size(); ++binding) {
		const auto& descriptorLayout = setLayout[binding];
		if (descriptorLayout.type == DescriptorType::UniformDynamicBuffer ||
			descriptorLayout.type == DescriptorType::StorageBufferDynamic) {
			for (size_t array = 0; array < descriptorLayout.arraySize; array++)
			{
				dynamicOffsets.push_back(static_cast<uint32_t>(m_data->resourceBindings.dynamicOffsets[set][binding][array]));
			}
		}
	}
	vkCmdBindDescriptorSets(m_vkHandle, bindPoint,
		m_data->currentPipelineLayout->get_layout(), set, 1, &m_data->allocatedDescriptorSets[set], static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
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
	assert(!m_data || !m_data->currentRenderpass);
	assert(!m_data || !m_data->currentFramebuffer);
	assert(!(imageBarrierCounts != 0) || (imageBarriers != nullptr));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].sType == VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	assert(!(imageBarrierCounts != 0) || (imageBarriers[0].image != VK_NULL_HANDLE));
	vkCmdPipelineBarrier(m_vkHandle, srcStages, dstStages, 0, barrierCount,
		globals, bufferBarrierCounts, bufferBarriers, imageBarrierCounts, imageBarriers);
}

void vulkan::CommandBuffer::barrier(VkPipelineStageFlags srcStages, VkAccessFlags srcAccess, VkPipelineStageFlags dstStages, VkAccessFlags dstAccess)
{
	assert(!m_data || !m_data->currentRenderpass);
	assert(!m_data || !m_data->currentFramebuffer);
	VkMemoryBarrier barrier{
		.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER,
		.srcAccessMask = srcAccess,
		.dstAccessMask = dstAccess
	};
	vkCmdPipelineBarrier(m_vkHandle, srcStages, dstStages, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}

void vulkan::CommandBuffer::image_barrier(const Image& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkPipelineStageFlags srcStages, VkAccessFlags srcAccessFlags, VkPipelineStageFlags dstStages, VkAccessFlags dstAccessFlags)
{
	assert(!m_data || !m_data->currentRenderpass);
	assert(!m_data || !m_data->currentFramebuffer);

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
	assert(!m_tiny);
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
	assert(!m_tiny);
	assert(!m_isCompute);
	assert(m_data->indexState.buffer != VK_NULL_HANDLE);
	if (flush_graphics()) {
		vkCmdDrawIndexed(m_vkHandle, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	}
	else {
		std::cout << "Could not flush render state, dropped draw call.\n";
	}
}

void vulkan::CommandBuffer::dispatch(uint32_t groupsX, uint32_t groupsY, uint32_t groupsZ)
{
	assert(!m_tiny);
	if (flush_compute()) {
		vkCmdDispatch(m_vkHandle, groupsX, groupsY, groupsZ);
	}
	else {
		std::cout << "Could not flush compute state, dropped dispatch.\n";
	}
}

void vulkan::CommandBuffer::trace_rays(uint32_t width, uint32_t height, uint32_t depth)
{
	assert(!m_tiny);
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
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	uint32_t inputAttachmentCount = m_data->currentRenderpass->get_num_input_attachments(m_data->pipelineState.subpassIndex);

	for (size_t i = startBinding; i < inputAttachmentCount; i++ ){
		if (m_data->resourceBindings.bindings[set][i].size() < 1) {
			m_data->resourceBindings.bindings[set][i].resize(1);
		}
		if (m_data->resourceBindings.bindingIds[set][i].size() < 1) {
			m_data->resourceBindings.bindingIds[set][i].resize(1);
		}
		if (m_data->resourceBindings.samplerIds[set][i].size() <= 1) {
			m_data->resourceBindings.samplerIds[set][i].resize(1);
		}
		auto& inputAttachmentReference = m_data->currentRenderpass->get_input_attachment(static_cast<uint32_t>(i), m_data->pipelineState.subpassIndex);
		if (inputAttachmentReference.attachment == VK_ATTACHMENT_UNUSED)
			continue;

		const auto* view = m_data->framebufferAttachments[inputAttachmentReference.attachment];
		auto& image = m_data->resourceBindings.bindings[set][i][0].image;
		assert(view);
		assert(view->get_image()->get_usage() & VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
		if (view->get_id() == m_data->resourceBindings.bindingIds[set][i][0] &&
			image.imageLayout == inputAttachmentReference.layout)
			continue;

		image.imageLayout = inputAttachmentReference.layout;
		image.imageView = view->get_image_view();
		m_data->resourceBindings.bindingIds[set][i][0] = view->get_id();
		m_data->resourceBindings.samplerIds[set][i][0] = 0;
		m_data->dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_acceleration_structure(uint32_t set, uint32_t binding, uint32_t arrayIndex, const AccelerationStructure& accelerationStructure)
{
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);

	if (m_data->resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (accelerationStructure.get_id() == m_data->resourceBindings.bindingIds[set][binding][arrayIndex])
		return;
	m_data->dirtyDescriptorSets.set(set);
	m_data->resourceBindings.bindingIds[set][binding][arrayIndex] = accelerationStructure.get_id();
	m_data->resourceBindings.bindings[set][binding][arrayIndex].accelerationStructure;
	m_data->resourceBindings.samplerIds[set][binding][arrayIndex] = 0;

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
void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, const ImageView& view)
{
	//assert(view.get_image()->get_usage() & VK_IMAGE_USAGE_SAMPLED_BIT);
	bind_texture(set, binding, arrayIndex, view.get_image_view(), view.get_image_view(), view.get_image()->get_info().layout, view.get_id());
}
void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Sampler* sampler)
{
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);


	if (m_data->resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (sampler->get_id() == m_data->resourceBindings.samplerIds[set][binding][arrayIndex])
		return;
	auto& image = m_data->resourceBindings.bindings[set][binding][arrayIndex].image;
	image.sampler = sampler->get_handle();
	m_data->dirtyDescriptorSets.set(set);
	m_data->resourceBindings.samplerIds[set][binding][arrayIndex] = sampler->get_id();
}

void vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, uint32_t arrayIndex, DefaultSampler sampler)
{
	bind_sampler(set, binding, arrayIndex, r_device.get_default_sampler(sampler));
}

void vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, uint32_t arrayIndex, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID)
{
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);


	if (m_data->resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	auto& image = m_data->resourceBindings.bindings[set][binding][arrayIndex].image;
	auto& id = m_data->resourceBindings.bindingIds[set][binding][arrayIndex];
	if (id == bindingID && image.imageLayout == layout)
		return;
	image.imageLayout = layout;
	image.imageView = floatView;
	id = bindingID;
	m_data->dirtyDescriptorSets.set(set);

}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size)
{
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);
	assert((buffer.get_usage() & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) && "Wrong buffer usage");
	if (m_data->resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	//const auto& shaderLayout = m_data->currentPipelineLayout->get_shader_layout();
	//const auto& setLayout = shaderLayout.descriptors[set].descriptors;
	//const auto& descriptorLayout = setLayout[binding];
	//bool dynamic = descriptorLayout.type == DescriptorType::UniformDynamicBuffer;
	bool dynamic = false;

	auto& bind = m_data->resourceBindings.bindings[set][binding][arrayIndex];
	if (buffer.get_id() == m_data->resourceBindings.bindingIds[set][binding][arrayIndex] && bind.buffer.range == size
		&& dynamic) {
		if (m_data->resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex ||
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] != offset) {
			m_data->dirtyDescriptorSetsDynamicOffsets.set(set);
			m_data->resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		}
	}
	else {
		assert(r_device.get_physical_device_properties().limits.maxUniformBufferRange >= size);
		bind.buffer = VkDescriptorBufferInfo{
			.buffer = buffer.get_handle(),
			.offset = 0,
			.range = Math::min(r_device.get_physical_device_properties().limits.maxUniformBufferRange,size)
		};
		if (dynamic) {
			if (m_data->resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex) {
				m_data->resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
			}
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		}
		else {
			bind.buffer.offset = offset;
		}
		m_data->resourceBindings.bindingIds[set][binding][arrayIndex] = buffer.get_id();
		m_data->resourceBindings.samplerIds[set][binding][arrayIndex] = 0;
		m_data->dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer)
{
	bind_uniform_buffer(set, binding, arrayIndex, buffer, 0, buffer.get_info().size);
}


void vulkan::CommandBuffer::bind_storage_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer, VkDeviceSize offset, VkDeviceSize size)
{
	assert(!m_tiny);
	assert(set < MAX_DESCRIPTOR_SETS);
	assert(binding < MAX_BINDINGS);
	assert((buffer.get_usage() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) && "Wrong buffer usage");
	if (m_data->resourceBindings.bindings[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindings[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.bindingIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.bindingIds[set][binding].resize(arrayIndex + 1ull);
	}
	if (m_data->resourceBindings.samplerIds[set][binding].size() <= arrayIndex) {
		m_data->resourceBindings.samplerIds[set][binding].resize(arrayIndex + 1ull);
	}
	const auto& shaderLayout = m_data->currentPipelineLayout->get_shader_layout();
	const auto& setLayout = shaderLayout.descriptors[set].descriptors;
	const auto& descriptorLayout = setLayout[binding];
	bool dynamic = descriptorLayout.type == DescriptorType::StorageBufferDynamic;
	auto& bind = m_data->resourceBindings.bindings[set][binding][arrayIndex];
	if (buffer.get_id() == m_data->resourceBindings.bindingIds[set][binding][arrayIndex] && bind.buffer.range == size
		&&dynamic) {
		if (m_data->resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex ||
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] != offset) {
			m_data->dirtyDescriptorSetsDynamicOffsets.set(set);
			m_data->resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		}
	}
	else {
		assert(r_device.get_physical_device_properties().limits.maxStorageBufferRange >= size);
		bind.buffer = VkDescriptorBufferInfo{
			.buffer = buffer.get_handle(),
			.offset = 0,
			.range = Math::min(r_device.get_physical_device_properties().limits.maxStorageBufferRange,size)
		};
		if (dynamic) {
			if (m_data->resourceBindings.dynamicOffsets[set][binding].size() <= arrayIndex) {
				m_data->resourceBindings.dynamicOffsets[set][binding].resize(static_cast<size_t>(arrayIndex) + 1);
			}
			m_data->resourceBindings.dynamicOffsets[set][binding][arrayIndex] = offset;
		} else{
			bind.buffer.offset = offset;
		}
		m_data->resourceBindings.bindingIds[set][binding][arrayIndex] = buffer.get_id();
		m_data->resourceBindings.samplerIds[set][binding][arrayIndex] = 0;
		m_data->dirtyDescriptorSets.set(set);
	}
}

void vulkan::CommandBuffer::bind_storage_buffer(uint32_t set, uint32_t binding, uint32_t arrayIndex, const Buffer& buffer)
{
	bind_storage_buffer(set, binding, arrayIndex, buffer, 0, buffer.get_info().size);
}
void vulkan::CommandBuffer::bind_vertex_buffer(uint32_t binding, const Buffer& buffer, VkDeviceSize offset, VkVertexInputRate inputRate)
{
	assert(!m_tiny);
	assert(binding < MAX_VERTEX_BINDINGS);
	assert(m_data->currentFramebuffer);


	if (buffer.get_handle() != m_data->vertexState.buffers[binding] || m_data->vertexState.offsets[binding] != offset)
		m_data->vertexState.dirty.set(binding);


	if (m_data->pipelineState.state.dynamic_vertex_input_binding_stride) {
		m_data->dynamicState.vertexStrides[binding] = inputRate;
	}
	else {
		if (m_data->pipelineState.inputRates[binding] != inputRate)
			m_data->invalidFlags.set(InvalidFlags::StaticVertex);
		m_data->pipelineState.inputRates.set(binding, inputRate);
	}
	m_data->vertexState.buffers[binding] = buffer.get_handle();
	m_data->vertexState.offsets[binding] = offset;
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
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_depth_test) {
		if (m_data->dynamicState.depth_test != static_cast<unsigned>(enabled)) {
			m_data->dynamicState.depth_test = static_cast<unsigned>(enabled);
			m_data->invalidFlags.set(InvalidFlags::DepthTest);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.depth_test != static_cast<unsigned>(enabled)) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.depth_test = static_cast<unsigned>(enabled);
		}
	}
}
void vulkan::CommandBuffer::bind_bindless_sets(uint32_t firstSet, VkDescriptorSet* sets, uint32_t setCount)
{
	for (size_t i = 0; i < setCount; ++i) {
		m_data->allocatedDescriptorSets[firstSet + i] = sets[i];
		m_data->bindlessDescriptorSets.set(firstSet + i);
		m_data->dirtyDescriptorSets.set(firstSet + i);
	}
}
void vulkan::CommandBuffer::set_depth_write(VkBool32 enabled) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_depth_write) {
		if (m_data->dynamicState.depth_write != static_cast<unsigned>(enabled)) {
			m_data->dynamicState.depth_write = static_cast<unsigned>(enabled);
			m_data->invalidFlags.set(InvalidFlags::DepthWrite);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.depth_write != enabled) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.depth_write = enabled;
		}
	}
}
void vulkan::CommandBuffer::set_depth_compare(VkCompareOp compare) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_depth_compare) {
		if (m_data->dynamicState.depth_compare != static_cast<unsigned>(compare)) {
			m_data->dynamicState.depth_compare = static_cast<unsigned>(compare);
			m_data->invalidFlags.set(InvalidFlags::DepthCompare);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.depth_compare != static_cast<unsigned>(compare)) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.depth_compare = static_cast<unsigned>(compare);
		}
	}
}
void vulkan::CommandBuffer::set_cull_mode(VkCullModeFlags cullMode) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_cull_mode) {
		if (m_data->dynamicState.cull_mode != static_cast<unsigned>(cullMode)) {
			m_data->dynamicState.cull_mode = static_cast<unsigned>(cullMode);
			m_data->invalidFlags.set(InvalidFlags::CullMode);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.cull_mode != static_cast<unsigned>(cullMode)) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.cull_mode = static_cast<unsigned>(cullMode);
		}
	}
}
void vulkan::CommandBuffer::set_front_face(VkFrontFace frontFace) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_front_face) {
		if (m_data->dynamicState.front_face != static_cast<unsigned>(frontFace)) {
			m_data->dynamicState.front_face = static_cast<unsigned>(frontFace);
			m_data->invalidFlags.set(InvalidFlags::FrontFace);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.front_face != static_cast<unsigned>(frontFace)) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.front_face = static_cast<unsigned>(frontFace);
		}
	}
}
void vulkan::CommandBuffer::set_topology(VkPrimitiveTopology topology) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_primitive_topology) {
		if (m_data->dynamicState.topology != static_cast<unsigned>(topology)) {
			m_data->dynamicState.topology = static_cast<unsigned>(topology);
			m_data->invalidFlags.set(InvalidFlags::FrontFace);
			m_data->invalidFlags.set(InvalidFlags::DynamicState);
		}
	}
	else {
		if (m_data->pipelineState.state.topology != static_cast<unsigned>(topology)) {
			m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
			m_data->pipelineState.state.topology = static_cast<unsigned>(topology);
		}
	}
}
void vulkan::CommandBuffer::set_blend_enable(VkBool32 enabled) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.blend_enable != static_cast<unsigned>(enabled)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.blend_enable = static_cast<unsigned>(enabled);
	}
}
void vulkan::CommandBuffer::set_src_color_blend(VkBlendFactor blendFactor) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.src_color_blend != static_cast<unsigned>(blendFactor)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.src_color_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_dst_color_blend(VkBlendFactor blendFactor) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dst_color_blend != static_cast<unsigned>(blendFactor)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.dst_color_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_src_alpha_blend(VkBlendFactor blendFactor) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.src_alpha_blend != static_cast<unsigned>(blendFactor)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.src_alpha_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::set_dst_alpha_blend(VkBlendFactor blendFactor) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.dst_alpha_blend != static_cast<unsigned>(blendFactor)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.dst_alpha_blend = static_cast<unsigned>(blendFactor);
	}
}
void vulkan::CommandBuffer::reset_pipeline_state() noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state != defaultPipelineState) {
		m_data->pipelineState.state = defaultPipelineState;
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
	}
}
void vulkan::CommandBuffer::set_polygon_mode(VkPolygonMode polygon_mode) noexcept {
	assert(!m_tiny);
	if (m_data->pipelineState.state.polygon_mode != static_cast<unsigned>(polygon_mode)) {
		m_data->invalidFlags.set(InvalidFlags::StaticPipeline);
		m_data->pipelineState.state.polygon_mode = static_cast<unsigned>(polygon_mode);
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
void vulkan::CommandBuffer::set_dynamic_vertex_input(bool state) noexcept
{
	assert(!m_tiny);
	assert(r_device.get_supported_extensions().vertex_input_dynamic_state && "Dynamic Vertex Input not supported but set");
	if(r_device.get_supported_extensions().vertex_input_dynamic_state)
		m_data->pipelineState.state.dynamic_vertex_input = state;
}
void vulkan::CommandBuffer::set_dynamic_vertex_input_binding_stride(bool state) noexcept
{
	assert(!m_tiny);
	m_data->pipelineState.state.dynamic_vertex_input_binding_stride = state;
}

void vulkan::CommandBuffer::bind_vertex_buffers() noexcept
{
	assert(!m_tiny);
	auto updateMask = m_data->vertexState.active & m_data->vertexState.dirty;
	Utility::for_each_bitrange(updateMask, [&](uint32_t binding, uint32_t range) {
		for (uint32_t i = binding; i < binding + range; i++)
			assert(m_data->vertexState.buffers[i] != VK_NULL_HANDLE);
		auto [buffers, offsets] = m_data->vertexState[binding];
		if (m_data->pipelineState.state.dynamic_vertex_input_binding_stride)
			vkCmdBindVertexBuffers2(m_vkHandle, binding, range, buffers, offsets, nullptr, m_data->dynamicState.vertexStrides.data() + binding);
		else
			vkCmdBindVertexBuffers(m_vkHandle,binding, range, buffers, offsets);
	});
	m_data->vertexState.update(updateMask);
}

void vulkan::CommandBuffer::invalidate_dynamic_state() noexcept
{
	assert(!m_tiny);
	m_data->invalidFlags.set(InvalidFlags::DynamicState);
	m_data->invalidFlags.set(InvalidFlags::Viewport);
	m_data->invalidFlags.set(InvalidFlags::Scissor);
	m_data->invalidFlags.set(InvalidFlags::DepthBiasEnable);
	m_data->invalidFlags.set(InvalidFlags::PrimitiveRestart);
	m_data->invalidFlags.set(InvalidFlags::RasterizerDiscard);
	m_data->invalidFlags.set(InvalidFlags::DepthBias);
	m_data->invalidFlags.set(InvalidFlags::Stencil);
	m_data->invalidFlags.set(InvalidFlags::CullMode);
	m_data->invalidFlags.set(InvalidFlags::FrontFace);
	m_data->invalidFlags.set(InvalidFlags::PrimitiveTopology);
	m_data->invalidFlags.set(InvalidFlags::DepthTest);
	m_data->invalidFlags.set(InvalidFlags::DepthWrite);
	m_data->invalidFlags.set(InvalidFlags::DepthCompare);
	m_data->invalidFlags.set(InvalidFlags::DepthBoundsTest);
	m_data->invalidFlags.set(InvalidFlags::StencilTest);
	m_data->invalidFlags.set(InvalidFlags::StencilOp);

}

void vulkan::CommandBuffer::update_dynamic_state() noexcept
{
	assert(!m_tiny);
	if (m_data->invalidFlags.get_and_clear(InvalidFlags::DynamicState)) {
		if (m_data->invalidFlags.get_and_clear(InvalidFlags::Viewport))
			vkCmdSetViewport(m_vkHandle, 0, 1, &m_data->viewport);
		if (m_data->invalidFlags.get_and_clear(InvalidFlags::Scissor))
			vkCmdSetScissor(m_vkHandle, 0, 1, &m_data->scissor);
		if (m_data->pipelineState.state.dynamic_depth_bias_enable && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthBiasEnable))
			vkCmdSetDepthBiasEnable(m_vkHandle, m_data->dynamicState.depth_bias_enable);
		if (m_data->pipelineState.state.dynamic_primitive_restart && m_data->invalidFlags.get_and_clear(InvalidFlags::PrimitiveRestart))
			vkCmdSetPrimitiveRestartEnable(m_vkHandle, m_data->dynamicState.primitive_restart);
		if (m_data->pipelineState.state.dynamic_rasterizer_discard && m_data->invalidFlags.get_and_clear(InvalidFlags::RasterizerDiscard))
			vkCmdSetRasterizerDiscardEnable(m_vkHandle, m_data->dynamicState.rasterizer_discard);
		if ((m_data->pipelineState.state.depth_bias_enable || (m_data->pipelineState.state.dynamic_depth_bias_enable && m_data->dynamicState.depth_bias_enable)) && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthBias))
			vkCmdSetDepthBias(m_vkHandle, m_data->dynamicState.depthBias, 0.0f, m_data->dynamicState.depthBiasSlope);
		if (m_data->pipelineState.state.stencil_test && m_data->invalidFlags.get_and_clear(InvalidFlags::Stencil)) {
			vkCmdSetStencilCompareMask(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_data->dynamicState.frontCompareMask);
			vkCmdSetStencilReference(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_data->dynamicState.frontReference);
			vkCmdSetStencilWriteMask(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, m_data->dynamicState.frontWriteMask);
			vkCmdSetStencilCompareMask(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_data->dynamicState.backCompareMask);
			vkCmdSetStencilReference(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_data->dynamicState.backReference);
			vkCmdSetStencilWriteMask(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, m_data->dynamicState.backWriteMask);
		}
		if (m_data->pipelineState.state.dynamic_cull_mode && m_data->invalidFlags.get_and_clear(InvalidFlags::CullMode)) {
			vkCmdSetCullMode(m_vkHandle, m_data->dynamicState.cull_mode);
		}
		if (m_data->pipelineState.state.dynamic_front_face && m_data->invalidFlags.get_and_clear(InvalidFlags::FrontFace)) {
			vkCmdSetFrontFace(m_vkHandle, static_cast<VkFrontFace>(m_data->dynamicState.front_face));
		}
		if (m_data->pipelineState.state.dynamic_primitive_topology && m_data->invalidFlags.get_and_clear(InvalidFlags::PrimitiveTopology)) {
			vkCmdSetPrimitiveTopology(m_vkHandle, static_cast<VkPrimitiveTopology>(m_data->dynamicState.topology));
		}
		if (m_data->pipelineState.state.dynamic_depth_test && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthTest)) {
			vkCmdSetDepthTestEnable(m_vkHandle, m_data->dynamicState.depth_test);
		}
		if (m_data->pipelineState.state.dynamic_depth_write && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthWrite)) {
			vkCmdSetDepthWriteEnable(m_vkHandle, m_data->dynamicState.depth_write);
		}
		if (m_data->pipelineState.state.dynamic_depth_compare && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthCompare)) {
			vkCmdSetDepthCompareOp(m_vkHandle, static_cast<VkCompareOp>(m_data->dynamicState.depth_compare));
		}
		if (m_data->pipelineState.state.dynamic_depth_bounds_test && m_data->invalidFlags.get_and_clear(InvalidFlags::DepthBoundsTest)) {
			vkCmdSetDepthBoundsTestEnable(m_vkHandle, m_data->dynamicState.depth_bound_test);
		}
		if (m_data->pipelineState.state.dynamic_stencil_test && m_data->invalidFlags.get_and_clear(InvalidFlags::StencilTest)) {
			vkCmdSetStencilTestEnable(m_vkHandle, m_data->dynamicState.stencil_test);
		}
		if (m_data->pipelineState.state.dynamic_stencil_op && m_data->invalidFlags.get_and_clear(InvalidFlags::StencilOp)) {
			vkCmdSetStencilOp(m_vkHandle, VK_STENCIL_FACE_FRONT_BIT, static_cast<VkStencilOp>(m_data->dynamicState.stencil_front_fail),
				static_cast<VkStencilOp>(m_data->dynamicState.stencil_front_pass), static_cast<VkStencilOp>(m_data->dynamicState.stencil_front_depth_fail),
				static_cast<VkCompareOp>(m_data->dynamicState.stencil_front_compare_op));
			vkCmdSetStencilOp(m_vkHandle, VK_STENCIL_FACE_BACK_BIT, static_cast<VkStencilOp>(m_data->dynamicState.stencil_back_fail),
				static_cast<VkStencilOp>(m_data->dynamicState.stencil_back_pass), static_cast<VkStencilOp>(m_data->dynamicState.stencil_back_depth_fail),
				static_cast<VkCompareOp>(m_data->dynamicState.stencil_back_compare_op));
		}
	}

}

void vulkan::CommandBuffer::update_push_constants() noexcept
{
	assert(!m_tiny);
	if (m_data->invalidFlags.get_and_clear(InvalidFlags::PushConstants)) {
		auto& pushConstants = m_data->currentPipelineLayout->get_shader_layout().pushConstantRange;

		if (pushConstants.stageFlags) {
			assert(pushConstants.offset == 0);
			//assert(false);
			
			vkCmdPushConstants(m_vkHandle, m_data->currentPipelineLayout->get_layout(), pushConstants.stageFlags, 0, pushConstants.size, m_data->resourceBindings.pushConstantData.data());
		}
	}
}

void vulkan::CommandBuffer::update_dynamic_vertex_state() noexcept
{
	assert(!m_tiny);
	if (m_data->pipelineState.state.dynamic_vertex_input && m_data->invalidFlags.get_and_clear(InvalidFlags::DynamicVertex)) {
		std::vector<VkVertexInputBindingDescription2EXT > bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription2EXT> attributeDescriptions;
		std::array<uint32_t, MAX_VERTEX_BINDINGS> offsets{};
		std::bitset<MAX_VERTEX_BINDINGS> bindings{};
		const auto& resourceLayout = m_data->pipelineState.program->get_pipeline_layout()->get_shader_layout();// .attributeElementCounts;
		attributeDescriptions.reserve(resourceLayout.inputs.count());
		Utility::for_each_bit(resourceLayout.inputs, [&](size_t location) {
			auto [format, binding] = m_data->pipelineState.attributes[location];
			assert(resourceLayout.attributeElementCounts[location] == format_element_count(format) && "given format conflicts with inputs described in shader");
			VkVertexInputAttributeDescription2EXT  desc{
				.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT,
				.location = static_cast<uint32_t>(location),
				.binding = binding,
				.format = format,
				//This design assumes that the elements are ordered
				.offset = offsets[binding]
			};
			bindings.set(binding);
			offsets[binding] += static_cast<uint32_t>(format_bytesize(format));
			attributeDescriptions.push_back(desc);
			});
		bindingDescriptions.reserve(bindings.count());
		Utility::for_each_bit(bindings, [&](size_t binding) {
			VkVertexInputBindingDescription2EXT  desc{
				.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT,
				.binding = static_cast<uint32_t>(binding),
				.stride = offsets[binding],
				.inputRate = m_data->pipelineState.inputRates[binding],
				.divisor = 1
			};
			bindingDescriptions.push_back(desc);
			});
		vkCmdSetVertexInputEXT(m_vkHandle, static_cast<uint32_t>(bindingDescriptions.size()), bindingDescriptions.data(),
			static_cast<uint32_t>(attributeDescriptions.size()), attributeDescriptions.data());
	}
}

void vulkan::CommandBuffer::push_constants(const void* data, VkDeviceSize offset, VkDeviceSize range)
{
	assert(!m_tiny);
	//TODO constant
	assert(offset + range <= 128);
	memcpy(m_data->resourceBindings.pushConstantData.data() + offset, data, range);
	m_data->invalidFlags.set(InvalidFlags::PushConstants);
}

void vulkan::CommandBuffer::bind_index_buffer(IndexState indexState)
{
	assert(!m_tiny);
	if (m_data->indexState == indexState)
		return;
	m_data->indexState = indexState;
	vkCmdBindIndexBuffer(m_vkHandle, indexState.buffer, indexState.offset, indexState.indexType);
}

void vulkan::CommandBuffer::init_viewport_and_scissor(const RenderpassCreateInfo& info)
{
	assert(!m_tiny);
	assert(m_data->currentFramebuffer);
	m_data->scissor = info.renderArea;
	auto [width, height] = m_data->currentFramebuffer->get_extent();
	m_data->scissor.offset.x = Math::min(m_data->scissor.offset.x, static_cast<int32_t>(width));
	m_data->scissor.offset.y = Math::min(m_data->scissor.offset.y, static_cast<int32_t>(height));
	m_data->scissor.extent.width = Math::min(width - m_data->scissor.offset.x, m_data->scissor.extent.width);
	m_data->scissor.extent.height = Math::min(height - m_data->scissor.offset.y, m_data->scissor.extent.height);

	m_data->viewport = VkViewport {
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
	assert(!m_tiny);
	m_data->currentRenderpass = r_device.request_render_pass(renderpassInfo);
	m_data->currentFramebuffer = r_device.request_framebuffer(renderpassInfo);
	m_data->pipelineState.subpassIndex = 0;
	m_data->pipelineState.compatibleRenderPass = r_device.request_compatible_render_pass(renderpassInfo);
	
	init_viewport_and_scissor(renderpassInfo);
	m_data->framebufferAttachments = {};
	std::array<VkClearValue, MAX_ATTACHMENTS + 1> clearValues{};
	uint32_t clearValueCount = static_cast<uint32_t>(renderpassInfo.colorAttachmentsCount);
	for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
		m_data->framebufferAttachments[i] = renderpassInfo.colorAttachmentsViews[i];
		if (renderpassInfo.clearAttachments.test(i)) {
			clearValues[i].color = renderpassInfo.clearColors[i];
		}
		if (renderpassInfo.colorAttachmentsViews[i]->get_image()->get_info().isSwapchainImage)
			m_swapchainTouched = true;
	}
	m_data->framebufferAttachments[renderpassInfo.colorAttachmentsCount] = renderpassInfo.depthStencilAttachment;
	if (renderpassInfo.depthStencilAttachment && renderpassInfo.opFlags.test(RenderpassCreateInfo::OpFlags::DepthStencilClear)) {
		clearValues[clearValueCount++].depthStencil = renderpassInfo.clearDepthStencil;
		
	}
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_data->currentRenderpass->get_render_pass(),
		.framebuffer = m_data->currentFramebuffer->get_handle(),
		.renderArea = m_data->scissor,
		.clearValueCount = clearValueCount,
		.pClearValues = clearValues.data(),
	};

	vkCmdBeginRenderPass(m_vkHandle, &renderPassBeginInfo, contents);
	m_data->currentContents = contents;
	begin_graphics();
}

void vulkan::CommandBuffer::end_render_pass()
{
	assert(!m_tiny);
	assert(m_data->currentFramebuffer && "No Framebuffer, forgot to begin render pass?");
	assert(m_data->currentRenderpass);
	assert(m_data->pipelineState.compatibleRenderPass);
	vkCmdEndRenderPass(m_vkHandle);
	m_data->currentFramebuffer = nullptr;
	m_data->currentRenderpass = nullptr;
	m_data->pipelineState.compatibleRenderPass = nullptr;
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
	assert(!m_tiny);
	assert(location < MAX_VERTEX_ATTRIBUTES);
	assert(m_data->currentFramebuffer);
	assert(binding < MAX_VERTEX_BINDINGS);
	m_data->vertexState.active.set(binding);
	auto [format_, binding_] = m_data->pipelineState.attributes[location];
	if (format_ == format && binding_ == binding)
		return; 

	m_data->pipelineState.attributes.bindings[location] = static_cast<uint8_t>(binding);
	m_data->pipelineState.attributes.formats[location] = format;
	if (m_data->pipelineState.state.dynamic_vertex_input) {
		m_data->invalidFlags.set(InvalidFlags::DynamicVertex);
	}
	else {
		m_data->invalidFlags.set(InvalidFlags::StaticVertex);
	}
}
void vulkan::CommandBuffer::set_scissor(VkRect2D scissor)
{
	assert(!m_tiny);
	m_data->invalidFlags.set(InvalidFlags::Scissor);
	m_data->invalidFlags.set(InvalidFlags::DynamicState);
	m_data->scissor = scissor;
}
void vulkan::CommandBuffer::set_viewport(VkViewport viewport)
{
	assert(!m_tiny);
	m_data->invalidFlags.set(InvalidFlags::Viewport);
	m_data->invalidFlags.set(InvalidFlags::DynamicState);
	m_data->viewport = viewport;
}
VkRect2D vulkan::CommandBuffer::get_scissor() const
{
	assert(!m_tiny);
	return m_data->scissor;
}
void vulkan::CommandBuffer::bind_program(Program* program)
{
	assert(!m_tiny);
	if (!program)
		return;
	if (m_data->pipelineState.program && (m_data->pipelineState.program->get_hash() == program->get_hash()))
		return;
	m_data->pipelineState.program = program;
	m_data->currentPipeline = VK_NULL_HANDLE;

	assert(( m_data->currentFramebuffer && program->get_shader(ShaderStage::Vertex)) ||
		   (!m_data->currentFramebuffer && program->get_shader(ShaderStage::Compute)));
	assert(program->get_pipeline_layout() != nullptr);

	m_data->invalidFlags.set(InvalidFlags::Pipeline);
	invalidate_dynamic_state();
	if (!m_data->currentPipelineLayout) {
		m_data->dirtyDescriptorSets.set();
		m_data->invalidFlags.set(InvalidFlags::PushConstants);
		m_data->currentPipelineLayout = program->get_pipeline_layout();
	}
	else if (auto* new_layout = program->get_pipeline_layout(); new_layout->get_hash() != m_data->currentPipelineLayout->get_hash()) {
		auto& newShaderLayout = new_layout->get_shader_layout();
		auto& oldShaderLayout = m_data->currentPipelineLayout->get_shader_layout();
		if (newShaderLayout.pushConstantRange != oldShaderLayout.pushConstantRange) {
			m_data->dirtyDescriptorSets.set();
			m_data->invalidFlags.set(InvalidFlags::PushConstants);
		}
		else {
			for (uint32_t i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
				if (new_layout->get_allocator(i) != m_data->currentPipelineLayout->get_allocator(i)) {
					decltype(m_data->dirtyDescriptorSets) update{};
					for (uint32_t j = 0; j < i; j++) {
						update.set(j);
					}
					m_data->dirtyDescriptorSets &= ~update;
					break;
				}
			}
		}
		m_data->currentPipelineLayout = new_layout;
	}
}
