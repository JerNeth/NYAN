#include "Renderer/MeshRenderer.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/Pipeline.h"
#include "VulkanWrapper/Shader.h"
#include "entt/entt.hpp"
#include "Utility/Exceptions.h"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const GBuffer& gbuffer) :
	Renderer(device, registry, renderManager, pass),
	m_gbuffer(gbuffer)
{
	r_pass.add_depth_stencil_attachment(m_gbuffer.depth, true);
	r_pass.add_attachment(m_gbuffer.albedo, true);
	r_pass.add_attachment(m_gbuffer.normal, true);
	r_pass.add_attachment(m_gbuffer.pbr, true);

	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
	{
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentPipeline);

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);

			PushConstants instance{
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
				.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
				.instanceId {0},
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
			};
			{
				auto view = r_registry.view<const MeshID, const InstanceId, const Deferred>();
				for (const auto& [entity, meshID, instanceId] : view.each()) {
					instance.instanceId = instanceId;
					render(pipelineBind, meshID, instance);
				}
			}
			{
				pipelineBind.set_cull_mode(VK_CULL_MODE_NONE);
				auto view = r_registry.view<const MeshID, const InstanceId, const DeferredDoubleSided>();
				for (const auto& [entity, meshID, instanceId] : view.each()) {
					instance.instanceId = instanceId;
					render(pipelineBind, meshID, instance);
				}
			}
		}
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentAlphaDiscardPipeline);

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);

			PushConstants instance{
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
				.instanceBinding {r_renderManager.get_instance_manager().get_binding()},
				.instanceId {0},
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
			};
			{
				auto view = r_registry.view<const MeshID, const InstanceId, const DeferredAlphaTest>();
				for (const auto& [entity, meshID, instanceId] : view.each()) {
					instance.instanceId = instanceId;
					render(pipelineBind, meshID, instance);
				}
			}
			{
				pipelineBind.set_cull_mode(VK_CULL_MODE_NONE);
				auto view = r_registry.view<const MeshID, const InstanceId, const DeferredDoubleSidedAlphaTest>();
				for (const auto& [entity, meshID, instanceId] : view.each()) {
					instance.instanceId = instanceId;
					render(pipelineBind, meshID, instance);
				}
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

nyan::ForwardMeshRenderer::ForwardMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting, const nyan::RenderResource::Id& depth) :
	Renderer(device, registry, renderManager, pass),
	m_lighting(lighting),
	m_depth(depth)
{
	r_pass.add_depth_attachment(m_depth);
	r_pass.add_attachment(m_lighting.specular);
	r_pass.add_attachment(m_lighting.diffuse);

	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_staticTangentPipeline);
			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);				
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
	staticTangentConfig.dynamicState.stencil_test_enable = VK_FALSE;

	r_pass.add_pipeline(staticTangentConfig, &m_staticTangentPipeline);
}


nyan::RTMeshRenderer::RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, nyan::RenderResource::Id rendertarget) :
	Renderer(device, registry, renderManager, pass),
	m_pipeline(device, generate_config()),
	m_rendertarget(rendertarget)
{
	r_pass.add_swapchain_write(Math::vec4{ 0.4f, 0.6f, 0.8f, 1.f }, nyan::Renderpass::Write::Type::Compute);
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
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.imageBinding {writeBind}, //0
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
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rgen")},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rchit")},
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_alpha_test_rahit")},
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