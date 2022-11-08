#include "Pipeline.h"
#include "LogicalDevice.h"
#include "Instance.h"
#include "Shader.h"
#include "DescriptorSet.h"
#include "RayTracePipeline.h"
#include "Utility/Exceptions.h"

vulkan::PipelineLayout2::PipelineLayout2(LogicalDevice& device, const std::vector<VkDescriptorSetLayout>& sets) :
	VulkanObject(device)
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

	if (auto result = vkCreatePipelineLayout(r_device.get_device(), &pipelineLayoutCreateInfo, r_device.get_allocator(), &m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}
vulkan::PipelineLayout2::~PipelineLayout2()
{
	if (m_handle != VK_NULL_HANDLE)
		vkDestroyPipelineLayout(r_device.get_device(), m_handle, r_device.get_allocator());
}

vulkan::PipelineCache::PipelineCache(LogicalDevice& device, const std::string& path) :
	VulkanObject(device),
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
			size = length;
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
	if (auto result = vkCreatePipelineCache(r_device.get_device(), &createInfo, r_device.get_allocator(), &m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}

vulkan::PipelineCache::~PipelineCache() noexcept
{
	size_t dataSize;
	VkResult result;
	std::vector<std::byte> data;
	do {
		vkGetPipelineCacheData(r_device.get_device(), m_handle, &dataSize, nullptr);
		data.resize(dataSize);
		result = vkGetPipelineCacheData(r_device.get_device(), m_handle, &dataSize, data.data());
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			vkDestroyPipelineCache(r_device.get_device(), m_handle, r_device.get_allocator());
			return;
		}
		else if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			vkDestroyPipelineCache(r_device.get_device(), m_handle, r_device.get_allocator());
			return;
		}
	} while (result != VK_SUCCESS);
	std::ofstream out(m_path, std::ios::binary);
	if (out.is_open()) {
		out.write(reinterpret_cast<char*>(data.data()), dataSize);
		out.close();
	}
	vkDestroyPipelineCache(r_device.get_device(), m_handle, r_device.get_allocator());
}

vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const GraphicsPipelineConfig& config) :
	VulkanObject(parent),
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_GRAPHICS),
	m_initialDynamicState(config.dynamicState)
{

	std::vector<VkPipelineShaderStageCreateInfo> shaders;
	for (uint32_t i = 0; i < config.shaderCount; i++)
		shaders.push_back(parent.get_shader_storage().get_instance(config.shaderInstances[i])->get_stage_info());

	for (auto shader1 = shaders.begin(); shader1 != shaders.end(); ++shader1)
		for (auto shader2 = shader1 + 1; shader2 != shaders.end(); ++shader2)
			assert(shader1->stage != shader2->stage);


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
		.viewportCount = 0,
		.pViewports {nullptr},
		.scissorCount = 0,
		.pScissors {nullptr} 
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = config.state.polygon_mode,
		.cullMode = config.dynamicState.cull_mode, //ignored since dynamic
		.frontFace = config.dynamicState.front_face, //ignored since dynamic
		.depthBiasEnable = config.dynamicState.depth_bias_enable, //ignored since dynamic
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
		.depthTestEnable = config.dynamicState.depth_test_enable, //ignored since dynamic
		.depthWriteEnable = config.dynamicState.depth_write_enable, //ignored since dynamic
		.depthCompareOp = config.dynamicState.depth_compare_op, //ignored since dynamic
		.depthBoundsTestEnable = config.dynamicState.depth_bounds_test_enable, //ignored since dynamic
		.stencilTestEnable = config.dynamicState.stencil_test_enable, //ignored since dynamic
		.front
		{ //ignored since dynamic
			.failOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_pass),
			.passOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_fail),
			.depthFailOp = static_cast<VkStencilOp>(config.dynamicState.stencil_front_depth_fail),
			.compareOp = static_cast<VkCompareOp>(config.dynamicState.stencil_front_compare_op),
			.compareMask = config.dynamicState.stencil_front_compare_mask,
			.writeMask = config.dynamicState.stencil_front_write_mask,
			.reference = config.dynamicState.stencil_front_reference,
		},
		.back
		{ //ignored since dynamic
			.failOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_pass),
			.passOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_fail),
			.depthFailOp = static_cast<VkStencilOp>(config.dynamicState.stencil_back_depth_fail),
			.compareOp = static_cast<VkCompareOp>(config.dynamicState.stencil_back_compare_op),
			.compareMask = config.dynamicState.stencil_back_compare_mask,
			.writeMask = config.dynamicState.stencil_back_write_mask,
			.reference = config.dynamicState.stencil_back_reference,
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

	std::vector<VkDynamicState> dynamicStates;
	dynamicStates.reserve(config.dynamicState.flags.count());
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::ViewportWithCount))
		dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::ScissorWithCount))
		dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::LineWidth))
		dynamicStates.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBias))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilCompareMask))
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilReference))
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilWriteMask))
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::CullMode))
		dynamicStates.push_back(VK_DYNAMIC_STATE_CULL_MODE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::FrontFace))
		dynamicStates.push_back(VK_DYNAMIC_STATE_FRONT_FACE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::PrimitiveTopology))
		dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthTestEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthWriteEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthCompareOp))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_COMPARE_OP);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBoundsTestEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilTestEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::StencilOp))
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_OP);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::DepthBiasEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::PrimitiveRestartEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE);
	if (config.dynamicState.flags.test(DynamicGraphicsPipelineState::DynamicState::RasterizerDiscardEnabled))
		dynamicStates.push_back(VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE);

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

	if (auto result = vkCreateGraphicsPipelines(parent.get_device(), parent.get_pipeline_cache(), 1, &graphicsPipelineCreateInfo, parent.get_allocator(), &m_handle);
		result != VK_SUCCESS && result != VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw Utility::VulkanException(result);
	}
}


vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const ComputePipelineConfig& config) : 
	VulkanObject(parent),
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_COMPUTE)
{

	assert(m_layout != VK_NULL_HANDLE);
	assert(config.shaderInstance != invalidShaderId);
	VkComputePipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // |VK_PIPELINE_CREATE_DISPATCH_BASE_BIT 
		.stage = parent.get_shader_storage().get_instance(config.shaderInstance)->get_stage_info(),
		.layout = m_layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	//Validate VUID-VkComputePipelineCreateInfo-flags-xxxxx
	assert(!(createInfo.flags & (VK_PIPELINE_CREATE_LIBRARY_BIT_KHR | 
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR | 
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV |
		VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV)));
	assert(createInfo.stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);
	
	if (auto result = vkCreateComputePipelines(parent.get_device(), parent.get_pipeline_cache(), 1, &createInfo, parent.get_allocator(), &m_handle);
		result != VK_SUCCESS && result != VK_PIPELINE_COMPILE_REQUIRED_EXT) {
		throw Utility::VulkanException(result);
	}
	
}

vulkan::Pipeline2::Pipeline2(LogicalDevice& parent, const RaytracingPipelineConfig& config) :
	VulkanObject(parent),
	m_layout(config.pipelineLayout),
	m_type(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR),
	m_initialDynamicState()
{
	const auto& rtFeatures = parent.get_physical_device().get_ray_tracing_pipeline_features();
	if (!rtFeatures.rayTracingPipeline) {
		Utility::log().location().message("Requested ray tracing pipeline on not supported hardware");
		throw Utility::FeatureNotSupportedException{};
	}
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
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline ray generation shader");
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

		if (!stageMap.contains(group.closestHitShader)) {
			auto* instance = parent.get_shader_storage().get_instance(group.closestHitShader);
			assert(instance);
			stageMap.emplace(group.closestHitShader, static_cast<uint32_t>(stageCreateInfos.size()));
			const auto& info = stageCreateInfos.emplace_back(instance->get_stage_info());
			if (info.stage & ~VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline closest hit shader");
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
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline any hit shader");
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
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline intersection shader");
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
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline miss shader");
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
				Utility::log().location().message("Invalid shadertype for ray tracing pipeline callable shader");
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
		.flags {},
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
		Utility::log().location().format("Requested Recursion Depth for Pipeline too high\n requested: {} \t supported {}", createInfo.maxPipelineRayRecursionDepth, rtProperties.maxRayRecursionDepth);
		createInfo.maxPipelineRayRecursionDepth = rtProperties.maxRayRecursionDepth;
	}
	
	if (auto result = vkCreateRayTracingPipelinesKHR(parent.get_device(), VK_NULL_HANDLE, parent.get_pipeline_cache(), 1, &createInfo, parent.get_allocator(), &m_handle);
		result != VK_SUCCESS && result != VK_PIPELINE_COMPILE_REQUIRED_EXT && result != VK_OPERATION_DEFERRED_KHR && result != VK_OPERATION_NOT_DEFERRED_KHR) {
		throw Utility::VulkanException(result);
	}
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
		vkDestroyPipeline(r_device.get_device(), pipeline, r_device.get_allocator());
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

void vulkan::GraphicsPipelineBind::set_depth_bias_enabled(bool enabled)
{
	vkCmdSetDepthBiasEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_depth_write_enabled(bool enabled)
{
	vkCmdSetDepthWriteEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_depth_test_enabled(bool enabled)
{
	vkCmdSetDepthTestEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_depth_bounds_test_enabled(bool enabled)
{
	vkCmdSetDepthBoundsTestEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_depth_compare_op(VkCompareOp compareOp)
{
	vkCmdSetDepthCompareOp(m_cmd, compareOp);
}

void vulkan::GraphicsPipelineBind::set_stencil_front_reference(uint32_t reference)
{
	vkCmdSetStencilReference(m_cmd, VK_STENCIL_FACE_FRONT_BIT, reference);
}

void vulkan::GraphicsPipelineBind::set_stencil_back_reference(uint32_t reference)
{
	vkCmdSetStencilReference(m_cmd, VK_STENCIL_FACE_BACK_BIT, reference);
}

void vulkan::GraphicsPipelineBind::set_stencil_front_write_mask(uint32_t mask)
{
	vkCmdSetStencilWriteMask(m_cmd, VK_STENCIL_FACE_FRONT_BIT, mask);
}

void vulkan::GraphicsPipelineBind::set_stencil_back_write_mask(uint32_t mask)
{
	vkCmdSetStencilWriteMask(m_cmd, VK_STENCIL_FACE_BACK_BIT, mask);
}

void vulkan::GraphicsPipelineBind::set_stencil_front_compare_mask(uint32_t mask)
{
	vkCmdSetStencilCompareMask(m_cmd, VK_STENCIL_FACE_FRONT_BIT, mask);
}

void vulkan::GraphicsPipelineBind::set_stencil_back_compare_mask(uint32_t mask)
{
	vkCmdSetStencilCompareMask(m_cmd, VK_STENCIL_FACE_BACK_BIT, mask);
}

void vulkan::GraphicsPipelineBind::set_stencil_test_enabled(bool enabled)
{
	vkCmdSetStencilTestEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_stencil_front_ops(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
{
	vkCmdSetStencilOp(m_cmd, VK_STENCIL_FACE_FRONT_BIT, failOp, passOp, depthFailOp, compareOp);
}

void vulkan::GraphicsPipelineBind::set_stencil_back_ops(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp)
{
	vkCmdSetStencilOp(m_cmd, VK_STENCIL_FACE_BACK_BIT, failOp, passOp, depthFailOp, compareOp);
}

void vulkan::GraphicsPipelineBind::set_cull_mode(VkCullModeFlags cullMode)
{
	vkCmdSetCullMode(m_cmd, cullMode);
}

void vulkan::GraphicsPipelineBind::set_front_face(VkFrontFace frontFace)
{
	vkCmdSetFrontFace(m_cmd, frontFace);
}

void vulkan::GraphicsPipelineBind::set_primitive_restart_enable(bool enabled)
{
	vkCmdSetPrimitiveRestartEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_rasterizer_discard_enable(bool enabled)
{
	vkCmdSetRasterizerDiscardEnable(m_cmd, enabled);
}

void vulkan::GraphicsPipelineBind::set_primitive_topology(VkPrimitiveTopology topology)
{
	vkCmdSetPrimitiveTopology(m_cmd, topology);
}

void vulkan::GraphicsPipelineBind::set_scissor(VkRect2D scissor)
{
	vkCmdSetScissor(m_cmd, 0, 1, &scissor);
}

void vulkan::GraphicsPipelineBind::set_scissor_with_count(uint32_t count, VkRect2D* scissor)
{
	vkCmdSetScissorWithCount(m_cmd, count, scissor);
}

void vulkan::GraphicsPipelineBind::set_viewport(VkViewport viewport)
{
	vkCmdSetViewport(m_cmd, 0, 1, &viewport);
}

void vulkan::GraphicsPipelineBind::set_viewport_with_count(uint32_t count, VkViewport* viewport)
{
	vkCmdSetViewportWithCount(m_cmd, count, viewport);
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

void vulkan::RaytracingPipelineBind::trace_rays(const vulkan::RTPipeline& pipeline, uint32_t width, uint32_t height, uint32_t depth)
{
	assert(pipeline.rgen_region());
	assert(pipeline.miss_region());
	assert(pipeline.hit_region());
	assert(pipeline.callable_region());
	vkCmdTraceRaysKHR(m_cmd, pipeline.rgen_region(),
		pipeline.miss_region(), pipeline.hit_region(),
		pipeline.callable_region(), width, height, depth);
}

void vulkan::RaytracingPipelineBind::trace_rays(const RTPipeline& pipeline, VkDeviceAddress address)
{
	assert(pipeline.rgen_region());
	assert(pipeline.miss_region());
	assert(pipeline.hit_region());
	assert(pipeline.callable_region());
	vkCmdTraceRaysIndirectKHR(m_cmd, pipeline.rgen_region(),
		pipeline.miss_region(), pipeline.hit_region(),
		pipeline.callable_region(), address);
}
