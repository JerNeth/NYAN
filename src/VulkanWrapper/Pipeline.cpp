#include "Pipeline.h"

#include "LogicalDevice.h"


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
Vulkan::PipelineLayout::PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout) :r_parent(parent) {
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
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
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
	VkPipelineColorBlendAttachmentState colorBlendAttachments[7];
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = compile.comatibleRenderPass->get_num_color_attachments(compile.subpassIndex),
		.pAttachments = colorBlendAttachments,
		.blendConstants{0.0f, 0.0f, 0.0f, 0.0f}
	};
	for (uint32_t i = 0; i < colorBlendStateCreateInfo.attachmentCount; i++) {
		auto& attachment = colorBlendAttachments[i];
		attachment = {};
		if (compile.comatibleRenderPass->get_color_attachment(compile.subpassIndex, i).attachment != VK_ATTACHMENT_UNUSED &&
			compile.program->get_pipeline_layout()->get_resourceLayout().outputs.test(i)) {
			attachment.colorWriteMask = (compile.state.color_write_mask >> (WRITE_MASK_BITS * i)) & ((1 << WRITE_MASK_BITS) - 1);
			attachment.blendEnable = compile.state.blend_enabled;
			if (attachment.blendEnable) {
				attachment.srcColorBlendFactor = compile.state.src_color_blend;
				attachment.dstColorBlendFactor = compile.state.dst_color_blend;
				attachment.colorBlendOp = compile.state.color_blend_op;
				attachment.srcAlphaBlendFactor = compile.state.src_alpha_blend;
				attachment.dstAlphaBlendFactor = compile.state.dst_alpha_blend;
				attachment.alphaBlendOp = compile.state.alpha_blend_op;
			}
		}
	}
	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front{},
		.back{},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	if (compile.comatibleRenderPass->has_depth_attachment(compile.subpassIndex)) {
		depthStencilStateCreateInfo.depthWriteEnable = compile.state.depth_test;
		depthStencilStateCreateInfo.stencilTestEnable = compile.state.depth_test;
		if (depthStencilStateCreateInfo.depthTestEnable = compile.state.depth_test) {
			depthStencilStateCreateInfo.front = {
				.failOp = compile.state.stencil_front_pass,
				.passOp = compile.state.stencil_front_fail,
				.depthFailOp = compile.state.stencil_front_depth_fail,
				.compareOp = compile.state.stencil_front_compare_op,
			};
			depthStencilStateCreateInfo.back = {
				.failOp = compile.state.stencil_back_pass,
				.passOp = compile.state.stencil_back_fail,
				.depthFailOp = compile.state.stencil_back_depth_fail,
				.compareOp = compile.state.stencil_back_compare_op,
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
		.topology = compile.state.topology,
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
		.polygonMode = compile.state.wireframe,
		.cullMode = compile.state.cull_mode,
		.frontFace = compile.state.front_face,
		.depthBiasEnable = compile.state.depth_bias_enable,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.0f,

	};
	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	for (uint32_t i = 0; i < NUM_SHADER_STAGES; i++) {
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
		//.layout = m_layout,
		//.renderPass = m_renderPass,
		.subpass = compile.subpassIndex,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if (auto result = vkCreateGraphicsPipelines(r_parent.m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, r_parent.m_allocator, &m_pipeline); result != VK_SUCCESS) {
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
}

template<typename VertexType, size_t numShaders>
void Vulkan::Pipeline::create_graphics_pipeline(std::array<Shader*, numShaders> shaders)
{

	std::array<VkPipelineShaderStageCreateInfo, numShaders> shaderStages;
	for (size_t i = 0; i < 2; i++) {
		shaderStages[i] = shaders[i]->get_create_info();
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
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.primitiveRestartEnable = VK_FALSE
	};

	VkViewport viewport{
		.x = 0.0f,
		.y = 0.0f,
		.width = m_width,
		.height = m_height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
	
	VkRect2D scissor{
		.offset = {0,0},
		.extent = VkExtent2D{static_cast<uint32_t>(m_width), static_cast<uint32_t>(m_height)}
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_NONE,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.0f,

	};

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
		.minSampleShading = 1.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = VK_FALSE, // Optional
		.alphaToOneEnable = VK_FALSE, // Optional
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
		.front{},
		.back{},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};
	VkPipelineColorBlendAttachmentState colorBlendAttachement{
		.blendEnable = VK_FALSE,
		.srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
		.colorBlendOp = VK_BLEND_OP_ADD,
		.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
		.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
		.alphaBlendOp = VK_BLEND_OP_ADD,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachement,
		.blendConstants{0.0f, 0.0f, 0.0f, 0.0f}
	};
	VkDynamicState dynamicStates[]{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates
	};

	


	
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = static_cast<uint32_t>(shaderStages.size()),
		.pStages = shaderStages.data(),
		.pVertexInputState = &vertexInputStateCreateInfo,
		.pInputAssemblyState = &inputAssemblyStateCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizationStateCreateInfo,
		.pMultisampleState = &multisampleStateCreateInfo,
		.pDepthStencilState = &depthStencilStateCreateInfo,
		.pColorBlendState = &colorBlendStateCreateInfo,
		.pDynamicState = nullptr, //&dynamicStateCreateInfo
		//.layout = m_layout,
		//.renderPass = m_renderPass,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if (auto result = vkCreateGraphicsPipelines(r_parent.m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, r_parent.m_allocator, &m_pipeline); result != VK_SUCCESS) {
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
