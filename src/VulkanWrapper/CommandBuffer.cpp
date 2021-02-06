#include "CommandBuffer.h"
#include "LogicalDevice.h"
Vulkan::CommandBuffer::CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle, Type type, uint32_t threadIdx) :
	r_device(parent),
	m_vkHandle(handle),
	m_threadIdx(threadIdx),
	m_type(type)
{
}

void Vulkan::CommandBuffer::begin_context()
{
	m_indexState = {};
	m_vertexState = {};
	m_dirtyDescriptorSets.set();
	m_currentPipeline = VK_NULL_HANDLE;
	m_currentPipelineLayout = nullptr;
	m_pipelineState.program = nullptr;
	m_resourceBindings.bindingIds = {};
	m_resourceBindings.samplerIds = {};
}

void Vulkan::CommandBuffer::begin_graphics()
{
	m_isCompute = false;
	begin_context();
}

void Vulkan::CommandBuffer::begin_compute()
{
	m_isCompute = true;
	begin_context();
}

bool Vulkan::CommandBuffer::flush_graphics()
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
			assert(false);
			vkCmdPushConstants(m_vkHandle, m_currentPipelineLayout->get_layout(), pushConstants.stageFlags, 0, pushConstants.size, nullptr);
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
	bind_vertex_buffers();
	return true;
}

bool Vulkan::CommandBuffer::flush_graphics_pipeline()
{
	m_currentPipeline = r_device.request_pipeline(m_pipelineState);
	return m_currentPipeline != VK_NULL_HANDLE;
}

void Vulkan::CommandBuffer::flush_descriptor_sets()
{
	auto& shaderLayout = m_currentPipelineLayout->get_shader_layout();
	
	auto setUpdate = shaderLayout.used & m_dirtyDescriptorSets;
	Utility::for_each_bit(setUpdate, [&](uint32_t set) {
		flush_descriptor_set(set);
	});
	m_dirtyDescriptorSets &= ~setUpdate;

	m_dirtyDescriptorSetsDynamicOffsets &= ~setUpdate;

	auto dynamicOffsetUpdates = shaderLayout.used & m_dirtyDescriptorSetsDynamicOffsets;
	Utility::for_each_bit(setUpdate, [&](uint32_t set) {
		rebind_descriptor_set(set);
	});
	m_dirtyDescriptorSetsDynamicOffsets &= ~dynamicOffsetUpdates;

}

void Vulkan::CommandBuffer::flush_descriptor_set(uint32_t set)
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


	auto [descriptorSet, allocated] =m_currentPipelineLayout->get_allocator(set)->find(m_threadIdx, hash);
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

void Vulkan::CommandBuffer::rebind_descriptor_set(uint32_t set)
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

void Vulkan::CommandBuffer::draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
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

bool Vulkan::CommandBuffer::swapchain_touched() const noexcept
{
	return m_swapchainTouched;
}

void Vulkan::CommandBuffer::touch_swapchain() noexcept
{
	m_swapchainTouched = true;
}

void Vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, const Sampler* sampler)
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

void Vulkan::CommandBuffer::bind_sampler(uint32_t set, uint32_t binding, DefaultSampler sampler)
{
	bind_sampler(set, binding, r_device.get_default_sampler(sampler));
}

void Vulkan::CommandBuffer::bind_texture(uint32_t set, uint32_t binding, VkImageView floatView, VkImageView integerView, VkImageLayout layout, Utility::UID bindingID)
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

VkCommandBuffer Vulkan::CommandBuffer::get_handle() const noexcept
{
	return m_vkHandle;
}

void Vulkan::CommandBuffer::end()
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

void Vulkan::CommandBuffer::bind_vertex_buffers() noexcept
{
	auto updateMask = m_vertexState.active & m_vertexState.dirty;
	Utility::for_each_bitrange(updateMask, [&](uint32_t binding, uint32_t range) {
		for (uint32_t i = binding; i < binding + range; i++)
			assert(m_vertexState.buffers[i] != VK_NULL_HANDLE);
		auto [buffers, offsets] = m_vertexState[binding];
		vkCmdBindVertexBuffers(m_vkHandle,binding, range, buffers, offsets);
	});
	m_vertexState.update(updateMask);
}

void Vulkan::CommandBuffer::bind_index_buffer(IndexState indexState)
{
	if (m_indexState == indexState)
		return;
	m_indexState = indexState;
	vkCmdBindIndexBuffer(m_vkHandle, indexState.buffer, indexState.offset, indexState.indexType);
}

void Vulkan::CommandBuffer::init_viewport_and_scissor(const RenderpassCreateInfo& info)
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

void Vulkan::CommandBuffer::begin_render_pass(const RenderpassCreateInfo& renderpassInfo, VkSubpassContents contents)
{
	m_currentFramebuffer = r_device.request_framebuffer(renderpassInfo);
	m_currentRenderpass = r_device.request_render_pass(renderpassInfo);
	m_pipelineState.subpassIndex = 0;
	m_pipelineState.compatibleRenderPass = r_device.request_compatible_render_pass(renderpassInfo);
	
	init_viewport_and_scissor(renderpassInfo);
	m_framebufferAttachments = {};
	std::array<VkClearValue, MAX_ATTACHMENTS + 1> clearValues;
	for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
		m_framebufferAttachments[i] = renderpassInfo.colorAttachmentsViews[i];
		if (renderpassInfo.clearAttachments.test(i)) {
			clearValues[i].color = renderpassInfo.clearColors[i];
		}
		if (renderpassInfo.colorAttachmentsViews[i]->get_image()->get_info().isSwapchainImage)
			m_swapchainTouched = true;
	}
	m_framebufferAttachments[renderpassInfo.colorAttachmentsCount] = renderpassInfo.depthStencilAttachment;
	if (renderpassInfo.depthStencilAttachment && renderpassInfo.opFlags.test(static_cast<size_t>(RenderpassCreateInfo::OpFlags::DepthStencilClear))) {
		clearValues[renderpassInfo.colorAttachmentsCount].depthStencil = renderpassInfo.clearDepthStencil;
	}
	VkRenderPassBeginInfo renderPassBeginInfo{
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_currentRenderpass->get_render_pass(),
		.framebuffer = m_currentFramebuffer->get_handle(),
		.renderArea = m_scissor,
		.clearValueCount = renderpassInfo.colorAttachmentsCount+1,
		.pClearValues = clearValues.data(),
	};

	vkCmdBeginRenderPass(m_vkHandle, &renderPassBeginInfo, contents);
	m_swapchainTouched = true;
	m_currentContents = contents;
	begin_graphics();
}

void Vulkan::CommandBuffer::end_render_pass()
{
	assert(m_currentFramebuffer);
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
void Vulkan::CommandBuffer::bind_program(Program* program)
{
	if (m_pipelineState.program == program)
		return;
	m_pipelineState.program = program;
	m_currentPipeline = VK_NULL_HANDLE;
	m_invalidFlags.set(InvalidFlags::DynamicState);
	m_invalidFlags.set(InvalidFlags::Pipeline);
	if (!program)
		return;
	assert(m_currentFramebuffer && program->get_shader(ShaderStage::Vertex));

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
					decltype(m_dirtyDescriptorSets) update;
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
