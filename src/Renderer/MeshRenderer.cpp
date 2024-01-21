#include "Renderer/MeshRenderer.h"

#include "entt/entt.hpp"
#include "Utility/Exceptions.h"

#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/Pipeline.hpp"
#include "VulkanWrapper/Shader.h"

#include "Renderer/RenderGraph.h"
#include "Renderer/RenderManager.h"

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
		.polygonMode {VK_POLYGON_MODE_FILL},
		.rasterizationSamples {VK_SAMPLE_COUNT_1_BIT},
		.logicOpEnable {VK_FALSE},
		.patchControlPoints {0},
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
	staticTangentConfig.dynamicState.stencilTestEnable = VK_TRUE;
	staticTangentConfig.dynamicState.stencilFrontReference = 0;
	staticTangentConfig.dynamicState.stencilFrontWriteMask = 0xFF;
	staticTangentConfig.dynamicState.stencilFrontCompareMask = 0xFF;
	staticTangentConfig.dynamicState.stencilFrontCompareOp = VK_COMPARE_OP_ALWAYS;
	staticTangentConfig.dynamicState.stencilFrontFail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencilFrontPass = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencilFrontDepthFail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencilBackReference = 0;
	staticTangentConfig.dynamicState.stencilBackWriteMask = 0xFF;
	staticTangentConfig.dynamicState.stencilBackCompareMask = 0xFF;
	staticTangentConfig.dynamicState.stencilBackCompareOp = VK_COMPARE_OP_ALWAYS;
	staticTangentConfig.dynamicState.stencilBackFail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencilBackPass = VK_STENCIL_OP_INCREMENT_AND_CLAMP;
	staticTangentConfig.dynamicState.stencilBackDepthFail = VK_STENCIL_OP_INCREMENT_AND_CLAMP;

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
					.accBinding { *r_renderManager.get_instance_manager().get_tlas_bind()},
			};
			auto view = r_registry.view<const MeshID, const InstanceId, const Forward>();
			for (const auto& [entity, meshID, instanceId] : view.each()) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshID, instance);
			}
			pipelineBind.set_depth_write_enabled(false);
			auto transparentView = r_registry.view<const MeshID, const InstanceId, const ForwardTransparent>();
			auto& instanceManager = r_renderManager.get_instance_manager();
			auto viewPos = r_renderManager.get_scene_manager().get_view_pos();
			std::vector<std::tuple<float, MeshID, InstanceId>> sorted{};
			for (const auto& [entity, meshID, instanceId] : transparentView.each()) {
				auto dist = (viewPos - instanceManager.get_instance(instanceId).transform.transformMatrix.col(3)).L2_norm();
				auto posInfo = std::lower_bound(sorted.begin(), sorted.end(), dist,
					[](const auto& val, float value)
					{
						return std::get<0>(val) < value;
					});
				sorted.insert(posInfo, std::tuple<float, MeshID, InstanceId>{dist, meshID, instanceId});
			}
			for (const auto& [dist, meshId, instanceId] : sorted) {
				instance.instanceId = instanceId;
				render(pipelineBind, meshId, instance);
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
		.polygonMode {VK_POLYGON_MODE_FILL},
		.rasterizationSamples {VK_SAMPLE_COUNT_1_BIT},
		.logicOpEnable {VK_FALSE},
		.patchControlPoints {0},
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
	staticTangentConfig.dynamicState.depthWriteEnable = VK_TRUE;
	staticTangentConfig.dynamicState.depthTestEnable = VK_TRUE;
	staticTangentConfig.dynamicState.stencilTestEnable = VK_FALSE;

	r_pass.add_pipeline(staticTangentConfig, &m_staticTangentPipeline);
}


nyan::RTMeshRenderer::RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting):
	Renderer(device, registry, renderManager, pass),
	m_lighting(lighting)
{
	r_pass.add_write(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	r_pass.add_write(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	pass.add_renderfunction(std::bind(&RTMeshRenderer::render, this, std::placeholders::_1, std::placeholders::_2), false);
}

void nyan::RTMeshRenderer::render(vulkan::CommandBuffer& cmd, nyan::Renderpass&)
{
	RTConfig rtConfig{
		.maxPathLength{m_maxPathLength},
		.antialiasing{m_antialising},
	};
	auto& diffuseResource = r_pass.get_graph().get_resource(m_lighting.diffuse);
	auto writeBindDiffuse = r_pass.get_write_bind(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	auto writeBindSpecular = r_pass.get_write_bind(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	assert(writeBindDiffuse != InvalidBinding);
	assert(writeBindSpecular != InvalidBinding);
	assert(diffuseResource.handle);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.diffuseImageBinding {writeBindDiffuse}, //0
		.specularImageBinding {writeBindSpecular}, //0
		.rngSeed {(*m_generator)()},
		.frameCount {++m_frameCount}
	};
	auto& rtPipeline = get_rt_pipeline(rtConfig);
	auto bind = cmd.bind_raytracing_pipeline(rtPipeline);
	bind.push_constants(constants);
	bind.trace_rays(rtPipeline, diffuseResource.handle->get_info().width, diffuseResource.handle->get_info().height, 1);
}

void nyan::RTMeshRenderer::reset()
{
	m_frameCount = 0;
}

void nyan::RTMeshRenderer::set_antialiasing(bool antialising)
{
	m_antialising = antialising;
}

void nyan::RTMeshRenderer::set_max_path_length(uint32_t maxPathLength)
{
	m_maxPathLength = maxPathLength;
}

vulkan::RTPipeline& nyan::RTMeshRenderer::get_rt_pipeline(const RTConfig& config)
{
	if (auto it = m_rtPipelines.find(config); it != m_rtPipelines.end()) {
		assert(it->second);
		return *it->second;
	}
	else {
		return *m_rtPipelines.emplace(config, std::make_unique< vulkan::RTPipeline>(r_device, generate_rt_config(config))).first->second;
	}
}

vulkan::RaytracingPipelineConfig nyan::RTMeshRenderer::generate_rt_config(const RTConfig& config)
{
	return vulkan::RaytracingPipelineConfig{
	.rgenGroups {
		vulkan::Group
		{
			.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rgen",
				vulkan::ShaderStorage::SpecializationConstant<uint32_t>{.name = "maxPathLength", .value = config.maxPathLength},
				vulkan::ShaderStorage::SpecializationConstant<uint32_t>{.name = "antialiasing", .value = config.antialiasing}
			)},
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
	.recursionDepth {1},
	.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}
