#include "Renderer/DeferredLighting.h"
#include "entt/entt.hpp"

nyan::DeferredLighting::DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd->bind_graphics_pipeline(m_deferredPipeline);
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
	auto readBind = r_pass.get_read_bind(0);	
	PushConstants constants{
		.imgBinding {readBind},
		.imgSampler {2},
	};
	bind.push_constants(constants);
	bind.draw(3, 1);
}

void nyan::DeferredLighting::create_pipeline()
{
	vulkan::GraphicsPipelineConfig pipelineConfig{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::defaultGraphicsPipelineState,
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {
		get_formats<nyan::Mesh>()
		//VK_FORMAT_R32G32B32_SFLOAT,
		//VK_FORMAT_R32G32_SFLOAT,
		//VK_FORMAT_R32G32B32_SFLOAT,
		//VK_FORMAT_R32G32B32_SFLOAT,
		//VK_FORMAT_R32G32B32_SFLOAT,
		//VK_FORMAT_R16G16_SFLOAT,
		//VK_FORMAT_R16G16B16_SFLOAT,
		//VK_FORMAT_R16G16B16_SFLOAT,
	},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("fullscreen_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("deferredLighting_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	pipelineConfig.dynamicState.depth_write_enable = VK_FALSE;
	pipelineConfig.dynamicState.depth_test_enable = VK_FALSE;
	pipelineConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	m_deferredPipeline = r_pass.add_pipeline(pipelineConfig);
}
