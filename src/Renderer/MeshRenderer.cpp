#include "Renderer/MeshRenderer.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderManager.h"
#include "entt/entt.hpp"
#include "CommandBuffer.h"
#include "Buffer.h"
#include "Pipeline.h"
#include "Utility/Exceptions.h"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
	{
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentPipeline);
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

			PushConstants instance{
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
				.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
				.instanceId {0},
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
			};
			auto view = r_registry.view<const MeshID, const InstanceId, const Deferred>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshID, instance);
			}
		}
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentAlphaDiscardPipeline);
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

			PushConstants instance{
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
				.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
				.instanceId {0},
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
			};
			auto view = r_registry.view<const MeshID, const InstanceId, const DeferredAlphaTest>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshID, instance);
			}
		}
	}, true);
}

void nyan::MeshRenderer::render(vulkan::GraphicsPipelineBind& pipelineBind, const MeshID& meshId, const PushConstants& instance)
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
	Renderer(device, registry, renderManager, pass)
{

	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentPipeline);
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
			PushConstants instance{
					.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
					.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
					//.instanceId {instanceId},
					.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
					.accBinding { *r_renderManager.get_instance_manager().get_tlas_bind()}
			};
			auto view = r_registry.view<const MeshID, const InstanceId, const Forward>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshID, instance);
			}
			pipelineBind.set_depth_write_enabled(false);
			auto transparentView = r_registry.view<const MeshID, const InstanceId, const ForwardTransparent>();
			for (const auto& [entity, meshID, instanceId] : transparentView.each()) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshID, instance);
			}
		}, true);
}

void nyan::ForwardMeshRenderer::render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const PushConstants& instance)
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


nyan::RTMeshRenderer::RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, nyan::RenderResource::Id rendertarget) :
	Renderer(device, registry, renderManager, pass),
	m_pipeline(device, generate_config()),
	m_rendertarget(rendertarget)
{
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_raytracing_pipeline(m_pipeline);
			
			render(pipelineBind);
			
		}, false);
}

void nyan::RTMeshRenderer::render(vulkan::RaytracingPipelineBind& bind)
{
	auto writeBind = r_pass.get_write_bind(m_rendertarget, nyan::Renderpass::Write::Type::Compute);
	assert(writeBind != InvalidBinding);
	PushConstants constants{
		.imageBinding {writeBind}, //0
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
	};
	bind.push_constants(constants);
	bind.trace_rays(m_pipeline, 1920, 1080, 1);
}

vulkan::RaytracingPipelineConfig nyan::RTMeshRenderer::generate_config()
{
	return vulkan::RaytracingPipelineConfig {
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI")},
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