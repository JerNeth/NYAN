#include "Renderer/DeferredLighting.h"
#include "entt/entt.hpp"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Sampler.h"
#include "Utility/Exceptions.h"
#include <future>

nyan::DeferredLighting::DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_deferredPipeline);
			VkViewport viewport{
			.x = 0,
			.y = 0,
			.width = static_cast<float>(r_device.get_swapchain_width()),
			.height = static_cast<float>(r_device.get_swapchain_height()),
			.minDepth = 0,
			.maxDepth = 1,
			};
			VkRect2D scissor{
			.offset {
				.x = static_cast<int32_t>(0),
				.y = static_cast<int32_t>(0),
			},
			.extent {
				.width = static_cast<uint32_t>(viewport.width),
				.height = static_cast<uint32_t>(viewport.height),
			}
			};
			pipelineBind.set_scissor(scissor);
			pipelineBind.set_viewport(viewport);

			
			render(pipelineBind);
		}, true);
}

void nyan::DeferredLighting::render(vulkan::GraphicsPipelineBind& bind)
{
	PushConstants constants{
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
		.albedoBinding {r_pass.get_read_bind("g_Albedo")},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind("g_Normal")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind("g_PBR")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind("g_Depth", nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind("g_Depth", nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
	};
	bind.push_constants(constants);
	bind.draw(3, 1);
}

void nyan::DeferredLighting::create_pipeline()
{
	vulkan::GraphicsPipelineConfig pipelineConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::GraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			vulkan::BlendAttachment {
				.blend_enable {VK_FALSE},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			},
			vulkan::BlendAttachment {
				.blend_enable {VK_FALSE},
				.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
			},
		},
	},
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("fullscreen_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("deferredLighting_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	pipelineConfig.dynamicState.depth_write_enable = VK_FALSE;
	pipelineConfig.dynamicState.depth_test_enable = VK_FALSE;
	pipelineConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	//pipelineConfig.dynamicState.stencil_test_enable = VK_TRUE;
	//pipelineConfig.dynamicState.stencil_front_reference = 0;
	//pipelineConfig.dynamicState.stencil_front_write_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_front_compare_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_front_compare_op = VK_COMPARE_OP_NOT_EQUAL;
	//pipelineConfig.dynamicState.stencil_front_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_front_pass = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_front_depth_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_reference = 0;
	//pipelineConfig.dynamicState.stencil_back_write_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_back_compare_mask = 0xFF;
	//pipelineConfig.dynamicState.stencil_back_compare_op = VK_COMPARE_OP_NOT_EQUAL;
	//pipelineConfig.dynamicState.stencil_back_fail = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_pass = VK_STENCIL_OP_KEEP;
	//pipelineConfig.dynamicState.stencil_back_depth_fail = VK_STENCIL_OP_KEEP;
	r_pass.add_pipeline(pipelineConfig, &m_deferredPipeline);
}

nyan::DeferredRayShadowsLighting::DeferredRayShadowsLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass),
	m_rtPipeline(create_pipeline(generate_config())),
	m_sbt(create_sbt(generate_config()))
{
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_raytracing_pipeline(m_rtPipeline);

			render(pipelineBind);

		}, false);
}

void nyan::DeferredRayShadowsLighting::render(vulkan::RaytracingPipelineBind& bind)
{
	auto writeBindDiffuse = r_pass.get_write_bind("DiffuseLighting", nyan::Renderpass::Write::Type::Compute);
	auto writeBindSpecular = r_pass.get_write_bind("SpecularLighting", nyan::Renderpass::Write::Type::Compute);
	assert(writeBindDiffuse != InvalidResourceId);
	assert(writeBindSpecular != InvalidResourceId);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.albedoBinding {r_pass.get_read_bind("g_Albedo")},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind("g_Normal")},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind("g_PBR")},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind("g_Depth", nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind("g_Depth", nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseImageBinding {writeBindDiffuse},
		.specularImageBinding {writeBindSpecular},
		
	};
	bind.push_constants(constants);
	bind.trace_rays(&m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callableRegion, 1920, 1080, 1);

}

vulkan::RaytracingPipelineConfig nyan::DeferredRayShadowsLighting::generate_config()
{
	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_deferred_rgen")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_shadows_rmiss")},
			},
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}

vulkan::PipelineId nyan::DeferredRayShadowsLighting::create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig)
{
	return r_device.get_pipeline_storage().add_pipeline(rayConfig);
}

vulkan::BufferHandle nyan::DeferredRayShadowsLighting::create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig)
{
	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(m_rtPipeline);


	const auto& rtProperties = r_device.get_physical_device().get_ray_tracing_pipeline_properties();

	auto handleSize{ rtProperties.shaderGroupHandleSize };
	auto groupCount{ rayConfig.rgenGroups.size() + rayConfig.hitGroups.size() + rayConfig.missGroups.size() + rayConfig.callableGroups.size() };
	std::vector<std::byte> handleData(handleSize * groupCount);
	if (auto result = vkGetRayTracingShaderGroupHandlesKHR(r_device, *pipeline, 0, static_cast<uint32_t>(groupCount), handleData.size(), handleData.data()); result != VK_SUCCESS) {
		assert(false);
		throw Utility::VulkanException(result);
	}
	auto handleStride = Utility::align_up(handleSize, rtProperties.shaderGroupHandleAlignment);
	uint32_t rgenCount{ static_cast<uint32_t>(rayConfig.rgenGroups.size()) };
	uint32_t hitCount{ static_cast<uint32_t>(rayConfig.hitGroups.size()) };
	uint32_t missCount{ static_cast<uint32_t>(rayConfig.missGroups.size()) };
	uint32_t callableCount{ static_cast<uint32_t>(rayConfig.callableGroups.size()) };

	assert(rgenCount == 1); //only one rgen per sbt, possibly per pipeline but idk
	m_rgenRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = Utility::align_up(handleStride, rtProperties.shaderGroupBaseAlignment),
		.size = Utility::align_up(rgenCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_hitRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(hitCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_missRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(missCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_callableRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(callableCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};

	assert(m_rgenRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_hitRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_missRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_callableRegion.stride <= rtProperties.maxShaderGroupStride);
	auto bufferSize = m_rgenRegion.size + m_hitRegion.size + m_missRegion.size + m_callableRegion.size;

	std::vector<std::byte> stridedHandles(bufferSize);
	std::vector<vulkan::InputData> inputData{
		vulkan::InputData{
			.ptr {stridedHandles.data()},
			.size {bufferSize},
		}
	};
	size_t offset{ 0 };
	size_t handleCount{ 0 };
	for (size_t i{ 0 }; i < rgenCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_rgenRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_rgenRegion.size;
	for (size_t i{ 0 }; i < hitCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_hitRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_hitRegion.size;
	for (size_t i{ 0 }; i < missCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_missRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_missRegion.size;
	for (size_t i{ 0 }; i < callableCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_callableRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_callableRegion.size;


	auto sbt = r_device.create_buffer(vulkan::BufferInfo{
		.size = bufferSize,
		.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
		}, inputData);

	auto buffAddr = sbt->get_address();

	if (rgenCount)
		m_rgenRegion.deviceAddress = buffAddr;
	if (hitCount)
		m_hitRegion.deviceAddress = buffAddr + m_rgenRegion.size;
	if (missCount)
		m_missRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size;
	if (callableCount)
		m_callableRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size + m_missRegion.size;

	return sbt;
}

nyan::LightComposite::LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_compositePipeline);
			VkViewport viewport{
			.x = 0,
			.y = 0,
			.width = static_cast<float>(r_device.get_swapchain_width()),
			.height = static_cast<float>(r_device.get_swapchain_height()),
			.minDepth = 0,
			.maxDepth = 1,
			};
			VkRect2D scissor{
			.offset {
				.x = static_cast<int32_t>(0),
				.y = static_cast<int32_t>(0),
			},
			.extent {
				.width = static_cast<uint32_t>(viewport.width),
				.height = static_cast<uint32_t>(viewport.height),
			}
			};
			pipelineBind.set_scissor(scissor);
			pipelineBind.set_viewport(viewport);


			render(pipelineBind);
		}, true);
}

void nyan::LightComposite::render(vulkan::GraphicsPipelineBind& bind)
{
	PushConstants constants{
		.specularBinding {r_pass.get_read_bind(0)},
		.specularSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseBinding {r_pass.get_read_bind(1)},
		.diffuseSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
	};
	bind.push_constants(constants);
	bind.draw(3, 1);
}

void nyan::LightComposite::create_pipeline()
{
	vulkan::GraphicsPipelineConfig pipelineConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::defaultGraphicsPipelineState,
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("fullscreen_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("composite_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	pipelineConfig.dynamicState.depth_write_enable = VK_FALSE;
	pipelineConfig.dynamicState.depth_test_enable = VK_FALSE;
	pipelineConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	r_pass.add_pipeline(pipelineConfig, &m_compositePipeline);
}
