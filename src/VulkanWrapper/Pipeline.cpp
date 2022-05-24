#include "Pipeline.h"
#include "Pipeline.h"
#include "Pipeline.h"
#include "LogicalDevice.h"

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
	std::array<VkDescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptorSets {VK_NULL_HANDLE};
	for (size_t i = 0; i < layout.used.count(); i++) {
		if (layout.used.test(i)) {
			m_descriptors[i] = r_device.request_descriptor_set_allocator(layout.descriptors[i]);
			descriptorSets[i] = m_descriptors[i]->get_layout();
		}
		else {
			assert(false);
		}
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
		std::vector<VkDescriptorUpdateTemplateEntry> entries;
		entries.reserve(MAX_BINDINGS);
		size_t offset{ 0 };

		auto& descriptorSetLayout = m_shaderLayout.descriptors[descriptorIdx].descriptors;
		for (size_t binding = 0; binding < descriptorSetLayout.size(); binding++) {
			const auto& descriptorLayout = descriptorSetLayout[binding];
			if (descriptorLayout.type == DescriptorType::Invalid)
				continue;
			VkDescriptorUpdateTemplateEntry entry{
				.dstBinding = static_cast<uint32_t>(binding),
				.dstArrayElement = 0,
				.descriptorCount = descriptorLayout.arraySize,
				.descriptorType = static_cast<VkDescriptorType>(descriptorLayout.type),
				.offset = offset,
				.stride = sizeof(ResourceBinding)
			};
			offset += descriptorLayout.arraySize * sizeof(ResourceBinding);
			entries.push_back(entry);
		}
		VkDescriptorUpdateTemplateCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO,
			.descriptorUpdateEntryCount = static_cast<uint32_t>(entries.size()),
			.pDescriptorUpdateEntries = entries.data(),
			.templateType = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_DESCRIPTOR_SET,
			.descriptorSetLayout = m_descriptors[descriptorIdx]->get_layout(),
			//Ignored since not VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR
			//.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
			//.pipelineLayout = m_layout,
			//.set = descriptorIdx,
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
vulkan::PipelineLayout2::PipelineLayout2(LogicalDevice& device, const std::vector<VkDescriptorSetLayout>& sets) :
	r_device(device)
{
	VkPushConstantRange range{
		.stageFlags = VK_SHADER_STAGE_ALL,
		.offset = 0,
		.size = device.get_physical_device().get_properties().limits.maxPushConstantsSize,
	};
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(sets.size()),
		.pSetLayouts = sets.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
	};

	if (auto result = vkCreatePipelineLayout(r_device.get_device(), &pipelineLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
		Utility::log_error(std::to_string((int)result));
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create pipeline layout, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create pipeline layout, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error");
		}
	}
}
vulkan::PipelineLayout2::~PipelineLayout2()
{
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(r_device.get_device(), m_layout, r_device.get_allocator());
}
vulkan::PipelineLayout2::operator VkPipelineLayout() const
{
	return m_layout;
}
VkPipelineLayout vulkan::PipelineLayout2::get_layout() const noexcept
{
	return m_layout;
}
vulkan::Pipeline::Pipeline(LogicalDevice& parent, const Program& program)
{
	assert(program.get_shader(vulkan::ShaderStage::Compute) != nullptr);
	VkComputePipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // |VK_PIPELINE_CREATE_DISPATCH_BASE_BIT 
		.stage = program.get_shader(vulkan::ShaderStage::Compute)->get_create_info(),
		.layout = program.get_pipeline_layout()->get_layout(),
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	vkCreateComputePipelines(parent.get_device(), parent.get_pipeline_cache(), 1, &createInfo, parent.get_allocator(), &m_pipeline);

}

vulkan::Pipeline::Pipeline(LogicalDevice& parent, const PipelineCompile& compile)
{


	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};
	std::array<VkDynamicState, 22> dynamicStates{
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
	if (compile.state.dynamic_depth_bias_enable) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
	}
	if (compile.state.dynamic_primitive_restart) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
	}
	if (compile.state.dynamic_rasterizer_discard) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
	}
	if (compile.state.dynamic_vertex_input) {
		dynamicStates[dynamicStateCreateInfo.dynamicStateCount++] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
	}
	std::array<VkPipelineColorBlendAttachmentState, MAX_ATTACHMENTS> colorBlendAttachments{};
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = compile.compatibleRenderPass->get_num_color_attachments(compile.subpassIndex),
		.pAttachments = colorBlendAttachments.data(),
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
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE,
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
		depthStencilStateCreateInfo.depthTestEnable = compile.state.depth_test;
		if (depthStencilStateCreateInfo.stencilTestEnable = compile.state.stencil_test; depthStencilStateCreateInfo.stencilTestEnable) {
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
	std::array<uint32_t, MAX_VERTEX_BINDINGS> offsets{};
	std::bitset<MAX_VERTEX_BINDINGS> bindings{};
	const auto& resourceLayout = compile.program->get_pipeline_layout()->get_shader_layout();// .attributeElementCounts;
	attributeDescriptions.reserve(resourceLayout.inputs.count());
	Utility::for_each_bit(resourceLayout.inputs, [&](size_t location) {
		auto [format, binding] = compile.attributes[location];
		assert(resourceLayout.attributeElementCounts[location] == format_element_count(format));
		VkVertexInputAttributeDescription desc{
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
		VkVertexInputBindingDescription desc{
			.binding = static_cast<uint32_t>(binding),
			.stride = offsets[binding],
			.inputRate = compile.inputRates[binding],
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
		.polygonMode = static_cast<VkPolygonMode>(compile.state.polygon_mode),
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
		.pTessellationState = nullptr,
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

	if (auto result = vkCreateGraphicsPipelines(parent.get_device(),parent.get_pipeline_cache(), 1, &graphicsPipelineCreateInfo, parent.get_allocator(), &m_pipeline);
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


VkPipeline vulkan::Pipeline::get_pipeline() const noexcept
{
	return m_pipeline;
}

vulkan::PipelineStorage::PipelineStorage(LogicalDevice& device) :
	r_device(device)
{
}
vulkan::PipelineStorage::~PipelineStorage()
{
	for (const auto& [compile, pipeline] : m_hashMap) {
		//assert(pipeline.get_pipeline() != VK_NULL_HANDLE);
		if (pipeline.get_pipeline() != VK_NULL_HANDLE)
			vkDestroyPipeline(r_device.get_device(), pipeline.get_pipeline(), r_device.get_allocator());
		else
			assert(false);
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

VkPipeline vulkan::PipelineStorage::request_pipeline(const Program& program)
{
	//TODO seperate Storage maybe, wasting ~200Bytes
	PipelineCompile compile{};
	memset(&compile, 0, sizeof(PipelineCompile));
	compile.program = &program; 
	const auto& [ret, _] = m_hashMap.try_emplace(compile, r_device, program);
	return ret->second.get_pipeline();
}

vulkan::PipelineCache::PipelineCache(LogicalDevice& device, const std::string& path) :
	r_parent(device),
	m_path(path)
{
	size_t size = 0;
	std::vector<std::byte> data;
	std::ifstream in(m_path, std::ios::binary);
	if (in.is_open()) {
		in.ignore(std::numeric_limits<std::streamsize>::max());
		std::streamsize length = in.gcount();
		if (length > 0) {
			in.clear();   //  Since ignore will have set eof.
			in.seekg(0, std::ios_base::beg);
			data.resize(length);
			in.read(reinterpret_cast<char*>(data.data()), length);
			in.close();
		}
		else {
			size = 0;
		}
	}
	VkPipelineCacheCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0,
		.initialDataSize = size,
		.pInitialData = data.data(),
	};
	if (auto result = vkCreatePipelineCache(r_parent.get_device(), &createInfo, r_parent.get_allocator(), &m_handle); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("PipelineCache: Out of memory");
		}
		else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("PipelineCache: Out of device memory");
		}
	}
}

vulkan::PipelineCache::~PipelineCache() noexcept
{
	size_t dataSize;
	VkResult result;
	std::vector<std::byte> data;
	do {
		vkGetPipelineCacheData(r_parent.get_device(), m_handle, &dataSize, nullptr);
		data.resize(dataSize);
		result = vkGetPipelineCacheData(r_parent.get_device(), m_handle, &dataSize, data.data());
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			vkDestroyPipelineCache(r_parent.get_device(), m_handle, r_parent.get_allocator());
			return;
		}
		else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			vkDestroyPipelineCache(r_parent.get_device(), m_handle, r_parent.get_allocator());
			return;
		}
	} while (result != VK_SUCCESS);
	std::ofstream out(m_path, std::ios::binary);
	if (out.is_open()) {
		out.write(reinterpret_cast<char*>(data.data()), dataSize);
		out.close();
	}
	vkDestroyPipelineCache(r_parent.get_device(), m_handle, r_parent.get_allocator());
}

VkPipelineCache vulkan::PipelineCache::get_handle() const noexcept
{
	return m_handle;
}

vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const GraphicsPipelineConfig& config) :
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_GRAPHICS),
	m_initialDynamicState(config.dynamicState)
{

	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	for (uint32_t i = 0; i < config.shaderCount; i++)
		shaders.push_back(parent.get_shader_storage().get_instance(config.shaderInstances[i])->get_stage_info());

	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	bindingDescriptions.reserve(config.vertexInputCount);
	attributeDescriptions.reserve(config.vertexInputCount);
	for (uint32_t location = 0; location < config.vertexInputCount; location++) {
		auto format = static_cast<VkFormat>(config.vertexInputFormats[location]);
		attributeDescriptions.push_back(VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(location),
			.binding = location,
			.format = format,
			.offset = 0
		});
		bindingDescriptions.push_back(
		VkVertexInputBindingDescription {
			.binding = static_cast<uint32_t>(location),
			.stride = static_cast<uint32_t>(format_bytesize(format)),
			.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		});
	}

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = config.vertexInputCount,
		.pVertexBindingDescriptions = bindingDescriptions.data(),
		.vertexAttributeDescriptionCount = config.vertexInputCount,
		.pVertexAttributeDescriptions = attributeDescriptions.data()
	};
	

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = config.dynamicState.primitive_topology,
		.primitiveRestartEnable = config.dynamicState.primitive_restart_enable
	};

	VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
		.pNext = nullptr,
		.patchControlPoints = config.state.patch_control_points
	};

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = config.state.polygon_mode,
		.cullMode = config.dynamicState.cull_mode,
		.frontFace = config.dynamicState.front_face,
		.depthBiasEnable = config.dynamicState.depth_bias_enable,
		.depthBiasConstantFactor = 0.f,
		.depthBiasClamp = 0.f,
		.depthBiasSlopeFactor = 0.f,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = config.state.rasterization_samples,
		.sampleShadingEnable = config.state.sample_shading,
		.minSampleShading = 0.0f, // Optional
		.pSampleMask = nullptr, // Optional
		.alphaToCoverageEnable = config.state.alpha_to_coverage, // Optional
		.alphaToOneEnable = config.state.alpha_to_one, // Optional
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = config.dynamicState.depth_test_enable,
		.depthWriteEnable = config.dynamicState.depth_write_enable,
		.depthCompareOp = config.dynamicState.depth_compare_op,
		.depthBoundsTestEnable = config.dynamicState.depth_bounds_test_enable,
		.stencilTestEnable = config.dynamicState.stencil_test_enable,
		.front
		{
			.failOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_pass),
			.passOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_fail),
			.depthFailOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_depth_fail),
			.compareOp = static_cast<VkCompareOp>(config.dynamicState.stencil_front_compare_op),
		},
		.back{
			.failOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_pass),
			.passOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_fail),
			.depthFailOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_depth_fail),
			.compareOp = static_cast<VkCompareOp>(config.dynamicState.stencil_back_compare_op),
		},
		.minDepthBounds = 0.0f,
		.maxDepthBounds = 1.0f,
	};

	std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{};
	colorBlendAttachments.reserve(config.renderingCreateInfo.colorAttachmentCount);
	for (uint32_t i = 0; i < config.renderingCreateInfo.colorAttachmentCount; i++) {
		colorBlendAttachments.push_back(VkPipelineColorBlendAttachmentState
			{
				.blendEnable = config.state.blendAttachments[i].blend_enable,
				.srcColorBlendFactor = config.state.blendAttachments[i].src_color_blend,
				.dstColorBlendFactor = config.state.blendAttachments[i].dst_color_blend,
				.colorBlendOp = config.state.blendAttachments[i].color_blend_op,
				.srcAlphaBlendFactor = config.state.blendAttachments[i].src_alpha_blend,
				.dstAlphaBlendFactor = config.state.blendAttachments[i].dst_alpha_blend,
				.alphaBlendOp = config.state.blendAttachments[i].alpha_blend_op,
				.colorWriteMask = config.state.blendAttachments[i].color_write_mask,
			});
	}
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = config.state.logic_op_enable,
		.logicOp = config.state.logic_op,
		.attachmentCount = config.renderingCreateInfo.colorAttachmentCount,
		.pAttachments = colorBlendAttachments.data(),
		.blendConstants{0.0f, 0.0f, 0.0f, 0.0f}
	};

	std::array dynamicStates{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
		VK_DYNAMIC_STATE_LINE_WIDTH,
		VK_DYNAMIC_STATE_DEPTH_BIAS,
		VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK,
		VK_DYNAMIC_STATE_STENCIL_REFERENCE,
		VK_DYNAMIC_STATE_STENCIL_WRITE_MASK,
		VK_DYNAMIC_STATE_CULL_MODE,
		VK_DYNAMIC_STATE_FRONT_FACE,
		VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY,
		VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE,
		VK_DYNAMIC_STATE_DEPTH_COMPARE_OP,
		VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE,
		VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE,
		VK_DYNAMIC_STATE_STENCIL_OP,
		VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE,
		VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE,
		VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	VkPipelineRenderingCreateInfo pipelineRenderingCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
		.viewMask = config.renderingCreateInfo.viewMask,
		.colorAttachmentCount = config.renderingCreateInfo.colorAttachmentCount,
		.pColorAttachmentFormats = config.renderingCreateInfo.colorAttachmentFormats.data(),
		.depthAttachmentFormat = config.renderingCreateInfo.depthAttachmentFormat,
		.stencilAttachmentFormat = config.renderingCreateInfo.stencilAttachmentFormat
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.pNext = &pipelineRenderingCreateInfo,
		.stageCount = static_cast<uint32_t>(shaders.size()),
		.pStages = shaders.data(),
		.pVertexInputState = &vertexInputStateCreateInfo,
		.pInputAssemblyState = &inputAssemblyStateCreateInfo,
		.pTessellationState = config.state.patch_control_points == 0 ? nullptr : &tessellationStateCreateInfo,
		.pViewportState = &viewportStateCreateInfo,
		.pRasterizationState = &rasterizationStateCreateInfo,
		.pMultisampleState = &multisampleStateCreateInfo,
		.pDepthStencilState = &depthStencilStateCreateInfo,
		.pColorBlendState = &colorBlendStateCreateInfo,
		.pDynamicState = &dynamicStateCreateInfo,
		.layout = config.pipelineLayout,
		.renderPass = VK_NULL_HANDLE,
		.subpass = 0,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};

	if (auto result = vkCreateGraphicsPipelines(parent.get_device(), parent.get_pipeline_cache(), 1, &graphicsPipelineCreateInfo, parent.get_allocator(), &m_pipeline);
		result != VK_SUCCESS) {

		Utility::log_error(std::to_string((int)result));
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
			throw std::runtime_error("VK: error");
		}
	}
}


vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const ComputePipelineConfig& config) : 
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_COMPUTE)
{
	VkComputePipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // |VK_PIPELINE_CREATE_DISPATCH_BASE_BIT 
		.stage = parent.get_shader_storage().get_instance(config.shaderInstance)->get_stage_info(),
		.layout = m_layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	vkCreateComputePipelines(parent.get_device(), parent.get_pipeline_cache(), 1, &createInfo, parent.get_allocator(), &m_pipeline);

}

vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const RaytracingPipelineConfig& config) :
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
	m_initialDynamicState()
{
	const auto& rtProperties = parent.get_physical_device().get_ray_tracing_pipeline_properties();


	std::vector<VkPipelineShaderStageCreateInfo> stageCreateInfos;
	std::vector<VkRayTracingShaderGroupCreateInfoKHR> groupCreateInfos;

	std::unordered_map<ShaderId, uint32_t> stageMap;
	stageMap[invalidShaderId] = VK_SHADER_UNUSED_KHR;
	for (const auto& group : config.rgenGroups) {
		assert(group.generalShader != invalidShaderId);
		assert(group.closestHitShader == invalidShaderId);
		assert(group.anyHitShader == invalidShaderId);
		assert(group.intersectionShader == invalidShaderId);

		if (!stageMap.contains(group.generalShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.generalShader);
			assert(instance);
			stageMap.emplace(group.generalShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_RAYGEN_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline ray generation shader"));
				assert(false);
				return;
			}
		}
		groupCreateInfos.push_back(
			VkRayTracingShaderGroupCreateInfoKHR
			{
				.sType { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR },
				.pNext { nullptr },
				.type { VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR },
				.generalShader { stageMap[group.generalShader] },
				.closestHitShader { VK_SHADER_UNUSED_KHR },
				.anyHitShader { VK_SHADER_UNUSED_KHR },
				.intersectionShader { VK_SHADER_UNUSED_KHR },
				.pShaderGroupCaptureReplayHandle { nullptr },
			});
	}
	for (const auto& group : config.hitGroups) {
		assert(group.generalShader == invalidShaderId);
		assert((group.closestHitShader != invalidShaderId)
		|| (group.anyHitShader != invalidShaderId)
		|| (group.intersectionShader != invalidShaderId));

		if (!stageMap.contains(group.closestHitShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.closestHitShader);
			assert(instance);
			stageMap.emplace(group.closestHitShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline closest hit shader"));
				assert(false);
				return;
			}
		}
		if (!stageMap.contains(group.anyHitShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.anyHitShader);
			assert(instance);
			stageMap.emplace(group.anyHitShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_ANY_HIT_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline any hit shader"));
				assert(false);
				return;
			}
		}
		if (!stageMap.contains(group.intersectionShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.intersectionShader);
			assert(instance);
			stageMap.emplace(group.intersectionShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_INTERSECTION_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline intersection shader"));
				assert(false);
				return;
			}
		}
		groupCreateInfos.push_back(
			VkRayTracingShaderGroupCreateInfoKHR
			{
				.sType { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR },
				.pNext { nullptr },
				.type { group.intersectionShader == invalidShaderId ? 
					VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR :
					VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR },
				.generalShader { VK_SHADER_UNUSED_KHR },
				.closestHitShader { stageMap[group.closestHitShader] },
				.anyHitShader { stageMap[group.anyHitShader] },
				.intersectionShader { stageMap[group.intersectionShader] },
				.pShaderGroupCaptureReplayHandle { nullptr },
			});
	}
	for (const auto& group : config.missGroups) {
		assert(group.generalShader != invalidShaderId);
		assert(group.closestHitShader == invalidShaderId);
		assert(group.anyHitShader == invalidShaderId);
		assert(group.intersectionShader == invalidShaderId);

		if (!stageMap.contains(group.generalShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.generalShader);
			assert(instance);
			stageMap.emplace(group.generalShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_MISS_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline miss shader"));
				assert(false);
				return;
			}
		}
		groupCreateInfos.push_back(
			VkRayTracingShaderGroupCreateInfoKHR
			{
				.sType { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR },
				.pNext { nullptr },
				.type { VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR },
				.generalShader { stageMap[group.generalShader] },
				.closestHitShader { VK_SHADER_UNUSED_KHR },
				.anyHitShader { VK_SHADER_UNUSED_KHR },
				.intersectionShader { VK_SHADER_UNUSED_KHR },
				.pShaderGroupCaptureReplayHandle { nullptr },
			});
	}
	for (const auto& group : config.callableGroups) {
		assert(group.generalShader != invalidShaderId);
		assert(group.closestHitShader == invalidShaderId);
		assert(group.anyHitShader == invalidShaderId);
		assert(group.intersectionShader == invalidShaderId);

		if (!stageMap.contains(group.generalShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.generalShader);
			assert(instance);
			stageMap.emplace(group.generalShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
				Utility::log(std::format("Invalid shadertype for ray tracing pipeline callable shader"));
				assert(false);
				return;
			}
		}
		groupCreateInfos.push_back(
			VkRayTracingShaderGroupCreateInfoKHR
			{
				.sType { VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR },
				.pNext { nullptr },
				.type { VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR },
				.generalShader { stageMap[group.generalShader] },
				.closestHitShader { VK_SHADER_UNUSED_KHR },
				.anyHitShader { VK_SHADER_UNUSED_KHR },
				.intersectionShader { VK_SHADER_UNUSED_KHR },
				.pShaderGroupCaptureReplayHandle { nullptr },
			});
	}

	assert(stageCreateInfos.size() < std::numeric_limits<uint32_t>::max());
	assert(groupCreateInfos.size() < std::numeric_limits<uint32_t>::max());
	VkRayTracingPipelineCreateInfoKHR createInfo
	{
		.sType { VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR },
		.pNext { nullptr },
		.stageCount { static_cast<uint32_t>(stageCreateInfos.size()) },
		.pStages { stageCreateInfos.data() },
		.groupCount { static_cast<uint32_t>(groupCreateInfos.size()) },
		.pGroups { groupCreateInfos.data() },
		.maxPipelineRayRecursionDepth { config.recursionDepth },
		.pLibraryInfo { nullptr },
		.pLibraryInterface { nullptr },
		.pDynamicState { nullptr },
		.layout { m_layout },
		.basePipelineHandle { VK_NULL_HANDLE },
		.basePipelineIndex { 0 },
	};
	if (createInfo.maxPipelineRayRecursionDepth > rtProperties.maxRayRecursionDepth) {
		Utility::log(std::format("Requested Recursion Depth for Pipeline too high\n requested: {} \t supported {}", createInfo.maxPipelineRayRecursionDepth, rtProperties.maxRayRecursionDepth));
		createInfo.maxPipelineRayRecursionDepth = rtProperties.maxRayRecursionDepth;
	}
	vkCreateRayTracingPipelinesKHR(parent.get_device(), VK_NULL_HANDLE, parent.get_pipeline_cache(), 1, &createInfo, parent.get_allocator(), &m_pipeline);
}

VkPipeline vulkan::Pipeline2::get_pipeline() const noexcept
{
	return m_pipeline;
}

vulkan::Pipeline2::operator VkPipeline() const noexcept
{
	return m_pipeline;
}

VkPipelineLayout vulkan::Pipeline2::get_layout() const noexcept
{
	return m_layout;
}

const vulkan::DynamicGraphicsPipelineState& vulkan::Pipeline2::get_dynamic_state() const noexcept
{
	return m_initialDynamicState;
}

vulkan::PipelineStorage2::PipelineStorage2(LogicalDevice& device) :
	r_device(device)
{
}

vulkan::PipelineStorage2::~PipelineStorage2()
{
	m_pipelines.for_each([this](Pipeline2& pipeline)
	{
		vkDestroyPipeline(r_device.get_device(), pipeline.get_pipeline(), r_device.get_allocator());
	});
}

vulkan::Pipeline2* vulkan::PipelineStorage2::get_pipeline(PipelineId pipelineId)
{
	return m_pipelines.get_ptr(pipelineId);
}
const vulkan::Pipeline2* vulkan::PipelineStorage2::get_pipeline(PipelineId pipelineId) const
{
	return m_pipelines.get_ptr(pipelineId);
}

vulkan::PipelineId vulkan::PipelineStorage2::add_pipeline(const ComputePipelineConfig& config)
{
	return static_cast<vulkan::PipelineId>( m_pipelines.emplace_intrusive(r_device, config));
}

vulkan::PipelineId vulkan::PipelineStorage2::add_pipeline(const GraphicsPipelineConfig& config)
{
	return static_cast<vulkan::PipelineId>(m_pipelines.emplace_intrusive(r_device, config));
}

vulkan::PipelineId vulkan::PipelineStorage2::add_pipeline(const RaytracingPipelineConfig& config)
{
	return static_cast<vulkan::PipelineId>(m_pipelines.emplace_intrusive(r_device, config));
}

vulkan::PipelineBind::PipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) :
	m_cmd(cmd),
	m_layout(layout),
	m_bindPoint(bindPoint)
{
}

void vulkan::PipelineBind::bind_descriptor_sets(uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptorSets,
	const std::vector<uint32_t>& dynamicOffsets)
{
	vkCmdBindDescriptorSets(m_cmd, m_bindPoint,m_layout, firstSet, static_cast<uint32_t>(descriptorSets.size()), descriptorSets.data(),
		static_cast<uint32_t>(dynamicOffsets.size()), dynamicOffsets.data());
}

vulkan::GraphicsPipelineBind::GraphicsPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) :
	PipelineBind(cmd, layout, bindPoint)
{
}

void vulkan::GraphicsPipelineBind::set_scissor(VkRect2D scissor)
{
	vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

void vulkan::GraphicsPipelineBind::set_viewport(VkViewport viewport)
{
	vkCmdSetViewport(m_cmd, 0, 1, &viewport);
}

void vulkan::GraphicsPipelineBind::bind_vertex_buffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets)
{
	vkCmdBindVertexBuffers(m_cmd, firstBinding, bindingCount, buffers, offsets);
}

void vulkan::GraphicsPipelineBind::bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(m_cmd, buffer, offset, indexType);
}

void vulkan::GraphicsPipelineBind::draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(m_cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void vulkan::GraphicsPipelineBind::draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance)
{
	vkCmdDrawIndexed(m_cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
	
}

vulkan::ComputePipelineBind::ComputePipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) :
	PipelineBind(cmd, layout, bindPoint)
{
}

void vulkan::ComputePipelineBind::dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	vkCmdDispatch(m_cmd, groupCountX, groupCountY, groupCountZ);
}

vulkan::RaytracingPipelineBind::RaytracingPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) :
	PipelineBind(cmd, layout, bindPoint)
{
}

void vulkan::RaytracingPipelineBind::trace_rays(const VkStridedDeviceAddressRegionKHR* raygenSBT, const VkStridedDeviceAddressRegionKHR* missSBT,
	const VkStridedDeviceAddressRegionKHR* hitSBT, const VkStridedDeviceAddressRegionKHR* callableSBT,
	uint32_t width, uint32_t height, uint32_t depth)
{
	assert(raygenSBT);
	assert(missSBT);
	assert(hitSBT);
	assert(callableSBT);
	vkCmdTraceRaysKHR(m_cmd, raygenSBT, missSBT, hitSBT, callableSBT, width, height, depth);
}
