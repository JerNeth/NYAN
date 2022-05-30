#include "Renderer/DeferredLighting.h"
#include "entt/entt.hpp"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Sampler.h"

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
	m_deferredPipeline = r_pass.add_pipeline(pipelineConfig);
}

nyan::LightComposite::LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass)
{
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd->bind_graphics_pipeline(m_compositePipeline);
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
	m_compositePipeline = r_pass.add_pipeline(pipelineConfig);
}
