#include "Renderer/MeshRenderer.h"

nyan::MeshRenderer::MeshRenderer(vulkan::LogicalDevice& device, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass) :
	r_device(device),
	r_shaderManager(shaderManager),
	r_meshManager(meshManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass& pass) 
		{

		});
}

void nyan::MeshRenderer::render(vulkan::GraphicsPipelineBind& pipelineBind, const MeshInstance& instance)
{
	auto& mesh = r_meshManager.get_static_tangent_mesh(instance.mesh_id);
	pipelineBind.bind_index_buffer(mesh.indexBuffer, mesh.indexOffset,  VK_INDEX_TYPE_UINT32);
	pipelineBind.bind_vertex_buffers(0, mesh.vertexBuffers.size(), mesh.vertexBuffers.data(), mesh.vertexOffsets.data());
	pipelineBind.push_constants(instance);
	pipelineBind.draw_indexed(mesh.indexCount, 1, mesh.firstIndex, mesh.vertexOffset, mesh.firstInstance);
}

void nyan::MeshRenderer::create_pipeline()
{
	vulkan::GraphicsPipelineConfig config{
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
	config.dynamicState.depth_write_enable = VK_FALSE;
	config.dynamicState.depth_test_enable = VK_FALSE;
	config.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	m_staticTangentPipeline = r_pass.add_pipeline(config);
}
