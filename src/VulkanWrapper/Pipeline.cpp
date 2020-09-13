#include "Pipeline.h"

#include "LogicalDevice.h"

Vulkan::PipelineState Vulkan::Pipeline::s_pipelineState = Vulkan::defaultPipelineState;
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
Vulkan::PipelineLayout::PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout) :r_parent(parent), m_resourceLayout(layout) {
	std::array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptorSets;
	for (int i = 0; i < descriptorSets.size(); i++) {
		auto allocator = r_parent.request_descriptor_set_allocator(layout.descriptors[i]);
		descriptorSets[i] = allocator->get_layout();
	}
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(layout.used.count()),
		.pSetLayouts = descriptorSets.data(),
		.pushConstantRangeCount = layout.pushConstantRange.size == 0? 0u: 1u,
		.pPushConstantRanges = &layout.pushConstantRange
	};

	if (auto result = vkCreatePipelineLayout(r_parent.m_device, &pipelineLayoutCreateInfo, r_parent.m_allocator, &m_layout); result != VK_SUCCESS) {
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
}
Vulkan::PipelineLayout::~PipelineLayout()
{
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(r_parent.m_device, m_layout, r_parent.m_allocator);
}

VkPipelineLayout Vulkan::PipelineLayout::get_layout() const
{
	return m_layout;
}

const  Vulkan::ShaderLayout& Vulkan::PipelineLayout::get_resourceLayout()
{
	return m_resourceLayout;
}

Vulkan::Pipeline::Pipeline(LogicalDevice& parent, const PipelineCompile& compile) :
	r_parent(parent)
{


	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	VkDynamicState dynamicStates[7]{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 3,
		.pDynamicStates = dynamicStates
	};
	if (compile.state.depth_bias_enable) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
	}
	if (compile.state.stencil_test) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;

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
		if (compile.compatibleRenderPass->get_color_attachment(compile.subpassIndex, i).attachment != VK_ATTACHMENT_UNUSED &&
			compile.program->get_pipeline_layout()->get_resourceLayout().outputs.test(i)) {
			attachment.colorWriteMask = (compile.state.color_write_mask >> (WRITE_MASK_BITS * i)) & ((1 << WRITE_MASK_BITS) - 1);
			if (attachment.blendEnable = compile.state.blend_enable) {
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
		if (depthStencilStateCreateInfo.depthTestEnable = compile.state.depth_test) {
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

	auto bindingDescriptions = Vertex::get_binding_descriptions();
	auto attributeDescriptions = Vertex::get_attribute_descriptions();

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
		.cullMode = VK_CULL_MODE_NONE,// static_cast<VkCullModeFlags>(compile.state.cull_mode),
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

	if (auto result = vkCreateGraphicsPipelines(r_parent.m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, r_parent.m_allocator, &m_pipeline);
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

Vulkan::Pipeline::~Pipeline()
{
	if (m_pipeline != VK_NULL_HANDLE)
		vkDestroyPipeline(r_parent.m_device, m_pipeline, r_parent.m_allocator);
}

VkPipeline Vulkan::Pipeline::get_pipeline()
{
	return m_pipeline;
}



Vulkan::Pipeline Vulkan::Pipeline::request_pipeline(LogicalDevice& parent, Program* program, Renderpass* compatibleRenderPass, uint32_t subpassIndex)
{
	return Pipeline(parent, { s_pipelineState, program, compatibleRenderPass, subpassIndex });
}

void Vulkan::Pipeline::reset_static_pipeline()
{
	s_pipelineState = defaultPipelineState;
}

void Vulkan::Pipeline::set_depth_write(bool depthWrite)
{
	s_pipelineState.depth_write = depthWrite;
}


void Vulkan::Pipeline::set_depth_test(bool depthTest)
{
	s_pipelineState.depth_test = depthTest;
}

void Vulkan::Pipeline::set_blend_enabled(bool blendEnabled)
{
	s_pipelineState.blend_enable = blendEnabled;
}

void Vulkan::Pipeline::set_cull_mode(VkCullModeFlags cullMode)
{
	s_pipelineState.cull_mode = cullMode;
}

void Vulkan::Pipeline::set_front_face(VkFrontFace frontFace)
{
	s_pipelineState.front_face = frontFace;
}

void Vulkan::Pipeline::set_depth_bias_enabled(bool depthBiasEnabled)
{
	s_pipelineState.depth_bias_enable = depthBiasEnabled;
}

void Vulkan::Pipeline::set_stencil_test_enabled(bool stencilTestEnabled)
{
	s_pipelineState.stencil_test = stencilTestEnabled;
}

void Vulkan::Pipeline::set_stencil_front_fail(VkStencilOp frontFail)
{
	s_pipelineState.stencil_front_fail = frontFail;
}

void Vulkan::Pipeline::set_stencil_front_pass(VkStencilOp frontPass)
{
	s_pipelineState.stencil_front_pass = frontPass;
}

void Vulkan::Pipeline::set_stencil_front_depth_fail(VkStencilOp frontDepthFail)
{
	s_pipelineState.stencil_front_depth_fail = frontDepthFail;
}

void Vulkan::Pipeline::set_stencil_front_compare_op(VkCompareOp frontCompareOp)
{
	s_pipelineState.stencil_front_compare_op = frontCompareOp;
}

void Vulkan::Pipeline::set_stencil_back_fail(VkStencilOp backFail)
{
	s_pipelineState.stencil_back_fail = backFail;
}

void Vulkan::Pipeline::set_stencil_back_pass(VkStencilOp backPass)
{
	s_pipelineState.stencil_back_pass = backPass;
}

void Vulkan::Pipeline::set_stencil_back_depth_fail(VkStencilOp backDepthFail)
{
	s_pipelineState.stencil_back_depth_fail = backDepthFail;
}

void Vulkan::Pipeline::set_stencil_back_compare_op(VkCompareOp backCompareOp)
{
	s_pipelineState.stencil_back_compare_op = backCompareOp;
}

void Vulkan::Pipeline::set_alpha_to_coverage(bool alphaToCoverage)
{
	s_pipelineState.alpha_to_coverage = alphaToCoverage;
}

void Vulkan::Pipeline::set_alpha_to_one(bool alphaToOne)
{
	s_pipelineState.alpha_to_one = alphaToOne;
}

void Vulkan::Pipeline::set_sample_shading(bool sampleShading)
{
	s_pipelineState.sample_shading = sampleShading;
}

void Vulkan::Pipeline::set_src_color_blend(VkBlendFactor srcColorBlend)
{
	s_pipelineState.src_color_blend = srcColorBlend;
}

void Vulkan::Pipeline::set_dst_color_blend(VkBlendFactor dstColorBlend)
{
	s_pipelineState.dst_color_blend = dstColorBlend;
}

void Vulkan::Pipeline::set_color_blend_op(VkBlendOp colorBlendOp)
{
	s_pipelineState.color_blend_op = colorBlendOp;
}

void Vulkan::Pipeline::set_src_alpha_blend(VkBlendFactor srcAlphaBlend)
{
	s_pipelineState.src_alpha_blend = srcAlphaBlend;
}

void Vulkan::Pipeline::set_dst_alpha_blend(VkBlendFactor dstAlphaBlend)
{
	s_pipelineState.dst_alpha_blend = dstAlphaBlend;
}

void Vulkan::Pipeline::set_alpha_blend_op(VkBlendOp alphaBlendOp)
{
	s_pipelineState.alpha_blend_op = alphaBlendOp;
}

void Vulkan::Pipeline::set_color_write_mask(VkColorComponentFlags writeMask, uint32_t colorAttachment)
{
	s_pipelineState.color_write_mask &= ~(((1 << WRITE_MASK_BITS) - 1)<<(colorAttachment*WRITE_MASK_BITS));
	s_pipelineState.color_write_mask |= writeMask << (colorAttachment*WRITE_MASK_BITS);
}

void Vulkan::Pipeline::set_primitive_restart(bool primitiveRestart)
{
	s_pipelineState.primitive_restart = primitiveRestart;
}

void Vulkan::Pipeline::set_topology(VkPrimitiveTopology primitiveTopology)
{
	s_pipelineState.topology = primitiveTopology;
}

void Vulkan::Pipeline::set_wireframe(VkPolygonMode wireframe)
{
	s_pipelineState.wireframe = wireframe;
}

void Vulkan::Pipeline::set_subgroup_control_size(bool controlSize)
{
	s_pipelineState.subgroup_control_size = controlSize;
}

void Vulkan::Pipeline::set_subgroup_full_group(bool fullGroup)
{
	s_pipelineState.subgroup_full_group = fullGroup;
}

void Vulkan::Pipeline::set_subgroup_min_size_log2(unsigned subgroupMinSize)
{
	assert(subgroupMinSize < (1 << 3));
	s_pipelineState.subgroup_min_size_log2 = subgroupMinSize;
}

void Vulkan::Pipeline::set_subgroup_max_size_log2(unsigned subgroupMaxSize)
{
	assert(subgroupMaxSize < (1 << 3));
	s_pipelineState.subgroup_max_size_log2 = subgroupMaxSize;
}

void Vulkan::Pipeline::set_conservative_raster(bool conservativeRaster)
{
	s_pipelineState.conservative_raster = conservativeRaster;
}