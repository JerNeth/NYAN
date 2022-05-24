#include "Renderer/MeshRenderer.h"
#include "entt/entt.hpp"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_shaderManager(shaderManager),
	r_meshManager(meshManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass& pass) 
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

		auto view = r_registry.view<const MeshID, const MaterialId, const InstanceId>();
		for (const auto& [entity, meshID, materialId, instanceId] : view.each()) {
			nyan::MeshInstance instance{
				.materialBinding {0},
				.materialId {materialId},
				.instanceBinding {1},
				.instanceId {instanceId},
				.view {Math::Mat<float, 4, 4, true>::look_at(Math::vec3{5, 5, 5}, Math::vec3{0,0,0}, Math::vec3{0, 0, 1})},
				.proj {Math::Mat<float, 4, 4, true>::perspectiveY(0.1, 10000, 40, 16 / 9.f) },
			};
			render(pipelineBind, meshID, instance);
		}
	});
}

void nyan::MeshRenderer::render(vulkan::GraphicsPipelineBind& pipelineBind, const MeshID& meshId, const MeshInstance& instance)
{
	auto& mesh = r_meshManager.get_static_tangent_mesh(meshId);
	pipelineBind.bind_index_buffer(mesh.indexBuffer, mesh.indexOffset,  VK_INDEX_TYPE_UINT32);
	pipelineBind.bind_vertex_buffers(0, mesh.vertexBuffers.size(), mesh.vertexBuffers.data(), mesh.vertexOffsets.data());

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
		r_shaderManager.get_shader_instance_id("staticTangent_vert"),
		r_shaderManager.get_shader_instance_id("staticTangent_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	staticTangentConfig.dynamicState.depth_write_enable = VK_TRUE;
	staticTangentConfig.dynamicState.depth_test_enable = VK_TRUE;
	staticTangentConfig.dynamicState.cull_mode = VK_CULL_MODE_BACK_BIT;
	m_staticTangentPipeline = r_pass.add_pipeline(staticTangentConfig);
}

nyan::RTMeshRenderer::RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_shaderManager(shaderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass& pass)
		{
			auto pipelineBind = cmd->bind_raytracing_pipeline(m_rtPipeline);
			render(pipelineBind);
			
		});
}

void nyan::RTMeshRenderer::render(vulkan::RaytracingPipelineBind& bind)
{
}

void nyan::RTMeshRenderer::create_pipeline()
{
	vulkan::RaytracingPipelineConfig rayConfig {
		.shaders {
			r_shaderManager.get_shader_instance_id("staticTangent_vert"),
			r_shaderManager.get_shader_instance_id("staticTangent_frag")
		},
		.groups {
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};

	m_rtPipeline = r_device.get_pipeline_storage().add_pipeline(rayConfig);
}