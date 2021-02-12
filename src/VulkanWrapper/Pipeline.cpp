#include "Pipeline.h"

#include "LogicalDevice.h"

vulkan::PipelineState vulkan::Pipeline::s_pipelineState = vulkan::defaultPipelineState;
/*
static std::vector<uint32_t> read_binary_file(const std::string& filename) {

	std::ifstream file(filename, std::ios::ate | std::ios::binary);
	if (!(file.is_open())) {
		throw std::runtime_error("Could not open file: \"" + filename + "\"");
	}

	auto fileSize = file.tellg();
	std::vector<uint32_t> buffer(fileSize/sizeof(uint32_t));

	file.seekg(0);
	file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	file.close();
	return buffer;
}
*/
vulkan::PipelineLayout::PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout) :r_device(parent), m_shaderLayout(layout) {
	m_hashValue = Utility::Hasher()(layout);
	std::array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptorSets;
	for (size_t i = 0; i < descriptorSets.size(); i++) {
		m_descriptors[i] = r_device.request_descriptor_set_allocator(layout.descriptors[i]);
		descriptorSets[i] = m_descriptors[i]->get_layout();
	}
	
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(layout.used.count()),
		.pSetLayouts = descriptorSets.data(),
		.pushConstantRangeCount = layout.pushConstantRange.size == 0u? 0u: 1u,
		.pPushConstantRanges = &layout.pushConstantRange
	};

	if (auto result = vkCreatePipelineLayout(r_device.m_device, &pipelineLayoutCreateInfo, r_device.m_allocator, &m_layout); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create pipeline layout, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create pipeline layout, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	create_update_template();
}
vulkan::PipelineLayout::~PipelineLayout()
{
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(r_device.get_device(), m_layout, r_device.get_allocator());
	for (uint32_t i = 0; i < MAX_DESCRIPTOR_SETS; i++) {
		if (m_updateTemplate[i] != VK_NULL_HANDLE)
			vkDestroyDescriptorUpdateTemplate(r_device.get_device(), m_updateTemplate[i], r_device.get_allocator());
	}
}

const VkPipelineLayout& vulkan::PipelineLayout::get_layout() const
{
	return m_layout;
}

const vulkan::ShaderLayout& vulkan::PipelineLayout::get_shader_layout() const
{
	return m_shaderLayout;
}

const vulkan::DescriptorSetAllocator* vulkan::PipelineLayout::get_allocator(size_t set) const
{
	assert(set < MAX_DESCRIPTOR_SETS);
	return m_descriptors[set];
}
vulkan::DescriptorSetAllocator* vulkan::PipelineLayout::get_allocator(size_t set)
{
	assert(set < MAX_DESCRIPTOR_SETS);
	return m_descriptors[set];
}

const VkDescriptorUpdateTemplate& vulkan::PipelineLayout::get_update_template(size_t set) const
{
	assert(set < MAX_DESCRIPTOR_SETS);
	return m_updateTemplate[set];
}

void vulkan::PipelineLayout::create_update_template()
{
	for (uint32_t descriptorIdx = 0; descriptorIdx < MAX_DESCRIPTOR_SETS; descriptorIdx++) {
		if (!m_shaderLayout.used.test(descriptorIdx))
			continue;
		std::array<VkDescriptorUpdateTemplateEntry, MAX_BINDINGS> entries;
		uint32_t updateCount{};

		auto& descriptor = m_shaderLayout.descriptors[descriptorIdx];
		Utility::for_each_bit(descriptor.uniformBuffer, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
				.offset = offsetof(ResourceBinding, buffer) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.storageBuffer, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.offset = offsetof(ResourceBinding, buffer) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.sampledBuffer, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
				.offset = offsetof(ResourceBinding, bufferView) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.imageSampler, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				.offset = (descriptor.fp.test(binding) ?offsetof(ResourceBinding, image.fp) :
					offsetof(ResourceBinding, image.integer)) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.separateImage, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.offset = (descriptor.fp.test(binding) ? offsetof(ResourceBinding, image.fp) :
					offsetof(ResourceBinding, image.integer)) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.seperateSampler, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.offset = offsetof(ResourceBinding, image.fp) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.storageImage, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.offset = (descriptor.fp.test(binding) ? offsetof(ResourceBinding, image.fp) :
					offsetof(ResourceBinding, image.integer)) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		Utility::for_each_bit(descriptor.inputAttachment, [&](uint32_t binding) {
			uint32_t arraySize = descriptor.arraySizes[binding];
			assert(updateCount < MAX_BINDINGS);
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = binding,
				.dstArrayElement = 0,
				.descriptorCount = arraySize,
				.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
				.offset = (descriptor.fp.test(binding) ? offsetof(ResourceBinding, image.fp) :
					offsetof(ResourceBinding, image.integer)) + sizeof(ResourceBinding) * binding,
				.stride = sizeof(ResourceBinding)
			};
			entries[updateCount++] = entry;
		});
		VkDescriptorUpdateTemplateCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			.descriptorUpdateEntryCount = updateCount,
			.pDescriptorUpdateEntries = entries.data(),
			.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			.descriptorSetLayout = m_descriptors[descriptorIdx]->get_layout(),
			//TODO handle compute and RT case
			.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			.pipelineLayout = m_layout,
			.set = descriptorIdx,
		};
		if (auto result = vkCreateDescriptorUpdateTemplate(r_device.m_device, &createInfo, r_device.m_allocator, &m_updateTemplate[descriptorIdx]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create DescriptorUpdateTemplate, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create DescriptorUpdateTemplate, out of device memory");
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
	}
}

vulkan::Pipeline::Pipeline(LogicalDevice& parent, const PipelineCompile& compile)
{


	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	std::array<VkDynamicState, 19> dynamicStates{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 3,
		.pDynamicStates = dynamicStates.data()
	};
	if (compile.state.depth_bias_enable) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
	}
	if (compile.state.stencil_test) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
	}
	if (parent.get_supported_extensions().extended_dynamic_state) {
		if (compile.state.dynamic_cull_mode) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_CULL_MODE_EXT;
		}
		if (compile.state.dynamic_front_face) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_FRONT_FACE_EXT;
		}
		if (compile.state.dynamic_primitive_topology) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY_EXT;
		}
		if (compile.state.dynamic_vertex_input_binding_stride) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE_EXT;
		}
		if (compile.state.dynamic_depth_test) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE_EXT;
		}
		if (compile.state.dynamic_depth_write) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE_EXT;
		}
		if (compile.state.dynamic_depth_compare) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP_EXT;
		}
		if (compile.state.dynamic_depth_bounds_test) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE_EXT;
		}
		if (compile.state.dynamic_stencil_test) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE_EXT;
		}
		if (compile.state.dynamic_stencil_op) {
			dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_OP_EXT;
		}
	}
	VkPipelineColorBlendAttachmentState colorBlendAttachments[MAX_ATTACHMENTS];
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = compile.compatibleRenderPass->get_num_color_attachments(compile.subpassIndex),
		.pAttachments = colorBlendAttachments,
		.blendConstants{0.0f, 0.0f, 0.0f, 0.0f}
	};
	for (uint32_t i = 0; i < colorBlendStateCreateInfo.attachmentCount; i++) {
		auto& attachment = colorBlendAttachments[i];
		attachment = {};
		if (compile.compatibleRenderPass->get_color_attachment(i, compile.subpassIndex).attachment != VK_ATTACHMENT_UNUSED &&
			compile.program->get_pipeline_layout()->get_shader_layout().outputs.test(i)) {
			attachment.colorWriteMask = (compile.state.color_write_mask >> (WRITE_MASK_BITS * i)) & ((1 << WRITE_MASK_BITS) - 1);
			if (attachment.blendEnable = compile.state.blend_enable; attachment.blendEnable) {
				attachment.srcColorBlendFactor = static_cast<VkBlendFactor>(compile.state.src_color_blend);
				attachment.dstColorBlendFactor = static_cast<VkBlendFactor>(compile.state.dst_color_blend);
				attachment.colorBlendOp = static_cast<VkBlendOp>(compile.state.color_blend_op);
				attachment.srcAlphaBlendFactor = static_cast<VkBlendFactor>(compile.state.src_alpha_blend);
				attachment.dstAlphaBlendFactor = static_cast<VkBlendFactor>(compile.state.dst_alpha_blend);
				attachment.alphaBlendOp = static_cast<VkBlendOp>(compile.state.alpha_blend_op);
			}
		}
	}
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = static_cast<VkCompareOp>(compile.state.depth_compare),
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front{},
		.back{},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	if (compile.compatibleRenderPass->has_depth_attachment(compile.subpassIndex)) {
		depthStencilStateCreateInfo.depthWriteEnable = compile.state.depth_test;
		depthStencilStateCreateInfo.stencilTestEnable = compile.state.depth_test;
		if (depthStencilStateCreateInfo.depthTestEnable = compile.state.depth_test; depthStencilStateCreateInfo.depthTestEnable) {
			depthStencilStateCreateInfo.front = {
				.failOp = static_cast<VkStencilOp>(compile.state.stencil_front_pass),
				.passOp = static_cast<VkStencilOp>(compile.state.stencil_front_fail),
				.depthFailOp = static_cast<VkStencilOp>(compile.state.stencil_front_depth_fail),
				.compareOp = static_cast<VkCompareOp>(compile.state.stencil_front_compare_op),
			};
			depthStencilStateCreateInfo.back = {
				.failOp = static_cast<VkStencilOp>(compile.state.stencil_back_pass),
				.passOp = static_cast<VkStencilOp>(compile.state.stencil_back_fail),
				.depthFailOp = static_cast<VkStencilOp>(compile.state.stencil_back_depth_fail),
				.compareOp = static_cast<VkCompareOp>(compile.state.stencil_back_compare_op),
			};
		}
	}

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	std::array<uint8_t, MAX_VERTEX_BINDINGS> offsets{};
	std::bitset<MAX_VERTEX_BINDINGS> bindings{};
	const auto& resourceLayout = compile.program->get_pipeline_layout()->get_shader_layout();// .attributeElementCounts;
	attributeDescriptions.reserve(resourceLayout.inputs.count());
	Utility::for_each_bit(resourceLayout.inputs, [&](uint32_t location) {
		auto [format, binding] = compile.attributes[location];
		assert(resourceLayout.attributeElementCounts[location] == format_element_count(format));
		VkVertexInputAttributeDescription desc{
					.location = location,
					.binding = binding,
					.format = format,
					//This design assumes that the elements are ordered
					.offset = offsets[binding]
		};
		bindings.set(binding);
		offsets[binding] += format_bytesize(format);
		attributeDescriptions.push_back(desc);
	});
	bindingDescriptions.reserve(bindings.count());
	Utility::for_each_bit(bindings, [&](uint32_t binding) {
		VkVertexInputBindingDescription desc{
			.binding = binding,
			.stride = offsets[binding],
			.inputRate = compile.inputRates[binding]
		};
		bindingDescriptions.push_back(desc);
	});

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size()),
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size()),
		.pVertexAttributeDescriptions = attributeDescriptions.data()
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = static_cast<VkPrimitiveTopology>(compile.state.topology),
		.primitiveRestartEnable = compile.state.primitive_restart
	};
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = compile.state.sample_shading,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = compile.state.alpha_to_coverage, // Optional
		.alphaToOneEnable = compile.state.alpha_to_one, // Optional
	};
	

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = static_cast<VkPolygonMode>(compile.state.wireframe),
		.cullMode = static_cast<VkCullModeFlags>(compile.state.cull_mode),
		.frontFace = static_cast<VkFrontFace>(compile.state.front_face),
		.depthBiasEnable = compile.state.depth_bias_enable,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.0f,
	};
	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	for (uint32_t i = 0; i < NUM_SHADER_STAGES; i++) {
		if(compile.program->get_shader(static_cast<ShaderStage>(i)))
			shaders.push_back(compile.program->get_shader(static_cast<ShaderStage>(i))->get_create_info());
	}
	


	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(shaders.size()),
		.pStages = shaders.data(),
		.pVertexInputState = &vertexInputStateCreateInfo,
		.pInputAssemblyState = &inputAssemblyStateCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizationStateCreateInfo,
		.pMultisampleState = &multisampleStateCreateInfo,
		.pDepthStencilState = &depthStencilStateCreateInfo,
		.pColorBlendState = &colorBlendStateCreateInfo,
		.pDynamicState = &dynamicStateCreateInfo,
		.layout = compile.program->get_pipeline_layout()->get_layout(),
		.renderPass = compile.compatibleRenderPass->get_render_pass(),
		.subpass = compile.subpassIndex,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if (auto result = vkCreateGraphicsPipelines(parent.get_device(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, parent.get_allocator(), &m_pipeline);
		result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create graphics pipeline, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create graphics pipeline, out of device memory");
		}
		if (result == VK_ERROR_INVALID_SHADER_NV) {
			throw std::runtime_error("VK: could not create graphics pipeline, invalid shader nv");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

//vulkan::Pipeline::~Pipeline() noexcept
//{
//	if (m_pipeline != VK_NULL_HANDLE)
//		vkDestroyPipeline(r_device.m_device, m_pipeline, r_device.m_allocator);
//}


//const vulkan::Pipeline& vulkan::Pipeline::operator=(vulkan::Pipeline&& other)
//{
//	this->r_device = other.r_device;
//	this->m_pipeline = other.m_pipeline;
//	other.m_pipeline = VK_NULL_HANDLE;
//}

VkPipeline vulkan::Pipeline::get_pipeline() const noexcept
{
	return m_pipeline;
}



vulkan::Pipeline vulkan::Pipeline::request_pipeline(LogicalDevice& parent, Program* program, Renderpass* compatibleRenderPass, Attributes attributes, InputRates inputRates, uint32_t subpassIndex)
{
	return Pipeline(parent, { s_pipelineState, program, compatibleRenderPass, attributes, inputRates, subpassIndex });
}

void vulkan::Pipeline::reset_static_pipeline()
{
	s_pipelineState = defaultPipelineState;
}

void vulkan::Pipeline::set_depth_write(bool depthWrite)
{
	s_pipelineState.depth_write = depthWrite;
}


void vulkan::Pipeline::set_depth_test(bool depthTest)
{
	s_pipelineState.depth_test = depthTest;
}

void vulkan::Pipeline::set_blend_enabled(bool blendEnabled)
{
	s_pipelineState.blend_enable = blendEnabled;
}

void vulkan::Pipeline::set_cull_mode(VkCullModeFlags cullMode)
{
	s_pipelineState.cull_mode = cullMode;
}

void vulkan::Pipeline::set_front_face(VkFrontFace frontFace)
{
	s_pipelineState.front_face = frontFace;
}

void vulkan::Pipeline::set_depth_bias_enabled(bool depthBiasEnabled)
{
	s_pipelineState.depth_bias_enable = depthBiasEnabled;
}

void vulkan::Pipeline::set_stencil_test_enabled(bool stencilTestEnabled)
{
	s_pipelineState.stencil_test = stencilTestEnabled;
}

void vulkan::Pipeline::set_stencil_front_fail(VkStencilOp frontFail)
{
	s_pipelineState.stencil_front_fail = frontFail;
}

void vulkan::Pipeline::set_stencil_front_pass(VkStencilOp frontPass)
{
	s_pipelineState.stencil_front_pass = frontPass;
}

void vulkan::Pipeline::set_stencil_front_depth_fail(VkStencilOp frontDepthFail)
{
	s_pipelineState.stencil_front_depth_fail = frontDepthFail;
}

void vulkan::Pipeline::set_stencil_front_compare_op(VkCompareOp frontCompareOp)
{
	s_pipelineState.stencil_front_compare_op = frontCompareOp;
}

void vulkan::Pipeline::set_stencil_back_fail(VkStencilOp backFail)
{
	s_pipelineState.stencil_back_fail = backFail;
}

void vulkan::Pipeline::set_stencil_back_pass(VkStencilOp backPass)
{
	s_pipelineState.stencil_back_pass = backPass;
}

void vulkan::Pipeline::set_stencil_back_depth_fail(VkStencilOp backDepthFail)
{
	s_pipelineState.stencil_back_depth_fail = backDepthFail;
}

void vulkan::Pipeline::set_stencil_back_compare_op(VkCompareOp backCompareOp)
{
	s_pipelineState.stencil_back_compare_op = backCompareOp;
}

void vulkan::Pipeline::set_alpha_to_coverage(bool alphaToCoverage)
{
	s_pipelineState.alpha_to_coverage = alphaToCoverage;
}

void vulkan::Pipeline::set_alpha_to_one(bool alphaToOne)
{
	s_pipelineState.alpha_to_one = alphaToOne;
}

void vulkan::Pipeline::set_sample_shading(bool sampleShading)
{
	s_pipelineState.sample_shading = sampleShading;
}

void vulkan::Pipeline::set_src_color_blend(VkBlendFactor srcColorBlend)
{
	s_pipelineState.src_color_blend = srcColorBlend;
}

void vulkan::Pipeline::set_dst_color_blend(VkBlendFactor dstColorBlend)
{
	s_pipelineState.dst_color_blend = dstColorBlend;
}

void vulkan::Pipeline::set_color_blend_op(VkBlendOp colorBlendOp)
{
	s_pipelineState.color_blend_op = colorBlendOp;
}

void vulkan::Pipeline::set_src_alpha_blend(VkBlendFactor srcAlphaBlend)
{
	s_pipelineState.src_alpha_blend = srcAlphaBlend;
}

void vulkan::Pipeline::set_dst_alpha_blend(VkBlendFactor dstAlphaBlend)
{
	s_pipelineState.dst_alpha_blend = dstAlphaBlend;
}

void vulkan::Pipeline::set_alpha_blend_op(VkBlendOp alphaBlendOp)
{
	s_pipelineState.alpha_blend_op = alphaBlendOp;
}

void vulkan::Pipeline::set_color_write_mask(VkColorComponentFlags writeMask, uint32_t colorAttachment)
{
	s_pipelineState.color_write_mask &= ~(((1 << WRITE_MASK_BITS) - 1)<<(colorAttachment*WRITE_MASK_BITS));
	s_pipelineState.color_write_mask |= writeMask << (colorAttachment*WRITE_MASK_BITS);
}

void vulkan::Pipeline::set_primitive_restart(bool primitiveRestart)
{
	s_pipelineState.primitive_restart = primitiveRestart;
}

void vulkan::Pipeline::set_topology(VkPrimitiveTopology primitiveTopology)
{
	s_pipelineState.topology = primitiveTopology;
}

void vulkan::Pipeline::set_wireframe(VkPolygonMode wireframe)
{
	s_pipelineState.wireframe = wireframe;
}

void vulkan::Pipeline::set_subgroup_control_size(bool controlSize)
{
	s_pipelineState.subgroup_control_size = controlSize;
}

void vulkan::Pipeline::set_subgroup_full_group(bool fullGroup)
{
	s_pipelineState.subgroup_full_group = fullGroup;
}

void vulkan::Pipeline::set_subgroup_min_size_log2(unsigned subgroupMinSize)
{
	assert(subgroupMinSize < (1 << 3));
	s_pipelineState.subgroup_min_size_log2 = subgroupMinSize;
}

void vulkan::Pipeline::set_subgroup_max_size_log2(unsigned subgroupMaxSize)
{
	assert(subgroupMaxSize < (1 << 3));
	s_pipelineState.subgroup_max_size_log2 = subgroupMaxSize;
}

void vulkan::Pipeline::set_conservative_raster(bool conservativeRaster)
{
	s_pipelineState.conservative_raster = conservativeRaster;
}
vulkan::PipelineStorage::PipelineStorage(LogicalDevice& device) :
	r_device(device)
{
}
vulkan::PipelineStorage::~PipelineStorage()
{
	for (const auto& [compile, pipeline] : m_hashMap) {
		assert(pipeline.get_pipeline() != VK_NULL_HANDLE);
		vkDestroyPipeline(r_device.get_device(), pipeline.get_pipeline(), r_device.get_allocator());
	}
}

VkPipeline vulkan::PipelineStorage::request_pipeline(const PipelineCompile& compile)
{
	
	const auto &[ret,_] = m_hashMap.try_emplace(compile, r_device, compile);
	return ret->second.get_pipeline();
	//try emplace would work but would result in unnecessary construction;
	/*if (const auto& ret = m_hashMap.find(compile); ret != m_hashMap.end()) {
		const auto& [res, _] = m_hashMap.emplace(compile, Pipeline(r_device,compile).get_pipeline());
		return res->second;
	}
	else {
		return ret->second;
	}*/
}
