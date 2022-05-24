#include "Renderer/MeshRenderer.h"
#include "entt/entt.hpp"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&) 
	{
		auto pipelineBind = cmd->bind_graphics_pipeline(m_staticTangentPipeline);
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

		auto view = r_registry.view<const MeshID, const InstanceId>();
		for (const auto& [entity, meshID, instanceId] : view.each()) {
			nyan::MeshInstance instance{
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
				.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
				.instanceId {instanceId},
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
			};
			render(pipelineBind, meshID, instance);
		}
	}, true);
}

void nyan::MeshRenderer::render(vulkan::GraphicsPipelineBind& pipelineBind, const MeshID& meshId, const MeshInstance& instance)
{
	auto& mesh = r_renderManager.get_mesh_manager().get_static_tangent_mesh(meshId);
	pipelineBind.bind_index_buffer(mesh.indexBuffer, mesh.indexOffset,  VK_INDEX_TYPE_UINT32);
	pipelineBind.bind_vertex_buffers(0u, mesh.vertexBuffers.size(), mesh.vertexBuffers.data(), mesh.vertexOffsets.data());

	pipelineBind.push_constants(instance);
	pipelineBind.draw_indexed(mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffset, mesh.firstInstance);
}

void nyan::MeshRenderer::create_pipeline()
{
	vulkan::GraphicsPipelineConfig staticTangentConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::alphaBlendedGraphicsPipelineState,
	.vertexInputCount = 4,
	.shaderCount = 2,
	.vertexInputFormats {
		VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32_SFLOAT,
		VK_FORMAT_R32G32B32_SFLOAT,
		VK_FORMAT_R32G32B32_SFLOAT,
	},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("staticTangent_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("staticTangent_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	staticTangentConfig.dynamicState.depth_write_enable = VK_TRUE;
	staticTangentConfig.dynamicState.depth_test_enable = VK_TRUE;
	staticTangentConfig.dynamicState.cull_mode = VK_CULL_MODE_BACK_BIT;
	m_staticTangentPipeline = r_pass.add_pipeline(staticTangentConfig);
}

nyan::RTMeshRenderer::RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass),
	m_rtPipeline(create_pipeline(generate_config())),
	m_sbt(create_sbt(generate_config()))
{
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd->bind_raytracing_pipeline(m_rtPipeline);
			
			render(pipelineBind);
			
		}, false);
}

void nyan::RTMeshRenderer::render(vulkan::RaytracingPipelineBind& bind)
{
	auto writeBind = r_pass.get_write_bind(0);
	assert(writeBind != InvalidResourceId);
	PushConstants constants{
		.imageBinding {writeBind},
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()},
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}
	};
	bind.push_constants(constants);
	bind.trace_rays(&m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callableRegion, 1920, 1080, 1);
}

vulkan::RaytracingPipelineConfig nyan::RTMeshRenderer::generate_config()
{
	return vulkan::RaytracingPipelineConfig {
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rgen")},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rchit")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rmiss")},
			},
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}

vulkan::PipelineId nyan::RTMeshRenderer::create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig)
{


	return r_device.get_pipeline_storage().add_pipeline(rayConfig);

}

vulkan::BufferHandle nyan::RTMeshRenderer::create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig)
{

	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(m_rtPipeline);

	const auto& rtProperties = r_device.get_physical_device().get_ray_tracing_pipeline_properties();
	auto handleSize{ rtProperties.shaderGroupHandleSize };
	auto groupCount{ rayConfig.rgenGroups.size() + rayConfig.hitGroups.size() + rayConfig.missGroups.size() + rayConfig.callableGroups.size() };
	std::vector<std::byte> handleData(handleSize * groupCount);
	auto result = vkGetRayTracingShaderGroupHandlesKHR(r_device, *pipeline, 0, static_cast<uint32_t>(groupCount), handleData.size(), handleData.data());
	assert(result == VK_SUCCESS);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Couldn't create SBT");
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

	std::vector<vulkan::InputData> inputData;
	for (size_t i{ 0 }; i < groupCount; ++i) {
		inputData.push_back(
			vulkan::InputData{
				.ptr {handleData.data() + i * handleSize},
				.size {handleSize},
				.stride {handleStride},
			}
		);
	}

	auto bufferSize = m_rgenRegion.size + m_hitRegion.size + m_missRegion.size + m_callableRegion.size;
	auto sbt = r_device.create_buffer(vulkan::BufferInfo{
		.size = bufferSize,
		.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
		}, inputData);

	auto buffAddr = sbt->get_address();

	if(rgenCount)
		m_rgenRegion.deviceAddress = buffAddr;
	if (hitCount)
		m_hitRegion.deviceAddress = buffAddr + m_rgenRegion.size;
	if (missCount)
		m_missRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size;
	if (callableCount)
		m_callableRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size + m_missRegion.size;

	return sbt;
}
