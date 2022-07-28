#include "Renderer/MeshRenderer.h"
#include "entt/entt.hpp"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "Utility/Exceptions.h"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&) 
	{
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

			auto view = r_registry.view<const MeshID, const InstanceId, const Deferred>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				nyan::MeshInstance instance{
					.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
					.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
					.instanceId {instanceId},
					.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
				};
				render(pipelineBind, meshID, instance);
			}
		}
		{
			auto pipelineBind = cmd->bind_graphics_pipeline(m_staticTangentAlphaDiscardPipeline);
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

			auto view = r_registry.view<const MeshID, const InstanceId, const DeferredAlphaTest>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				nyan::MeshInstance instance{
					.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
					.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
					.instanceId {instanceId},
					.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
				};
				render(pipelineBind, meshID, instance);
			}
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
	.state = vulkan::GraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			vulkan::defaultBlendAttachment,
			vulkan::defaultBlendAttachment,
			vulkan::defaultBlendAttachment,
		}
		},
	.vertexInputCount = get_num_formats<nyan::Mesh>(),
	.shaderCount = 2,
	.vertexInputFormats {
		get_formats<nyan::Mesh>()
	},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("staticTangent_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("deferredTangent_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	staticTangentConfig.dynamicState.depth_write_enable = VK_TRUE;
	staticTangentConfig.dynamicState.depth_test_enable = VK_TRUE;
	staticTangentConfig.dynamicState.cull_mode = VK_CULL_MODE_BACK_BIT;
	staticTangentConfig.dynamicState.stencil_test_enable = VK_TRUE;
	staticTangentConfig.dynamicState.stencil_front_reference = 0;
	staticTangentConfig.dynamicState.stencil_front_write_mask = 0xFF;
	staticTangentConfig.dynamicState.stencil_front_compare_mask = 0xFF;
	staticTangentConfig.dynamicState.stencil_front_compare_op = VK_COMPARE_OP_ALWAYS;
	staticTangentConfig.dynamicState.stencil_front_fail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencil_front_pass = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencil_front_depth_fail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencil_back_reference = 0;
	staticTangentConfig.dynamicState.stencil_back_write_mask = 0xFF;
	staticTangentConfig.dynamicState.stencil_back_compare_mask = 0xFF;
	staticTangentConfig.dynamicState.stencil_back_compare_op = VK_COMPARE_OP_ALWAYS;
	staticTangentConfig.dynamicState.stencil_back_fail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencil_back_pass = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencil_back_depth_fail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;

	r_pass.add_pipeline(staticTangentConfig, &m_staticTangentPipeline);

	staticTangentConfig.shaderInstances[1] = r_renderManager.get_shader_manager().get_shader_instance_id("deferredTangentAlphaDiscard_frag");
	r_pass.add_pipeline(staticTangentConfig, &m_staticTangentAlphaDiscardPipeline);
}

nyan::ForwardMeshRenderer::ForwardMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
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

			auto view = r_registry.view<const MeshID, const InstanceId, const Forward>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				nyan::MeshInstance instance{
					.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
					.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
					.instanceId {instanceId},
					.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
					.accBinding { *r_renderManager.get_instance_manager().get_tlas_bind()}
				};
				render(pipelineBind, meshID, instance);
			}
			pipelineBind.set_depth_write_enabled(false);
			auto transparentView = r_registry.view<const MeshID, const InstanceId, const ForwardTransparent>();
			for (const auto& [entity, meshID, instanceId] : transparentView.each()) {
				nyan::MeshInstance instance{
					.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
					.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
					.instanceId {instanceId},
					.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
					.accBinding { *r_renderManager.get_instance_manager().get_tlas_bind()}
				};
				render(pipelineBind, meshID, instance);
			}
		}, true);
}

void nyan::ForwardMeshRenderer::render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const MeshInstance& instance)
{
	auto& mesh = r_renderManager.get_mesh_manager().get_static_tangent_mesh(meshId);
	bind.bind_index_buffer(mesh.indexBuffer, mesh.indexOffset, VK_INDEX_TYPE_UINT32);
	bind.bind_vertex_buffers(0u, mesh.vertexBuffers.size(), mesh.vertexBuffers.data(), mesh.vertexOffsets.data());

	bind.push_constants(instance);
	bind.draw_indexed(mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffset, mesh.firstInstance);
}

void nyan::ForwardMeshRenderer::create_pipeline()
{

	vulkan::GraphicsPipelineConfig staticTangentConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::GraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {
			vulkan::alphaBlendAttachment,
			vulkan::alphaBlendAttachment,
		}
		},
	.vertexInputCount = get_num_formats<nyan::Mesh>(),
	.shaderCount = 2,
	.vertexInputFormats {
		get_formats<nyan::Mesh>()
	},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("staticTangent_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("forwardTangent_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	staticTangentConfig.dynamicState.depth_write_enable = VK_TRUE;
	staticTangentConfig.dynamicState.depth_test_enable = VK_TRUE;
	staticTangentConfig.dynamicState.cull_mode = VK_CULL_MODE_BACK_BIT;
	staticTangentConfig.dynamicState.stencil_test_enable = VK_FALSE;

	r_pass.add_pipeline(staticTangentConfig, &m_staticTangentPipeline);
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
	auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	assert(writeBind != InvalidResourceId);
	PushConstants constants{
		.imageBinding {writeBind}, //0
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
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
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_shadows_rmiss")},
			},
		},
		.recursionDepth {2},
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
