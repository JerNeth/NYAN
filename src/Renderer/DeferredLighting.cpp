#include "Renderer/DeferredLighting.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Sampler.h"
#include "Utility/Exceptions.h"
#include "entt/entt.hpp"

nyan::DeferredLighting::DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
	nyan::RenderResource::Id albedoRead, nyan::RenderResource::Id normalRead, nyan::RenderResource::Id pbrRead, nyan::RenderResource::Id depthRead,
	nyan::RenderResource::Id stencilRead) :
	Renderer(device, registry, renderManager, pass),
	m_albedoRead (albedoRead),
	m_normalRead (normalRead),
	m_pbrRead (pbrRead),
	m_depthRead	(depthRead),
	m_stencilRead (stencilRead)
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
		.albedoBinding {r_pass.get_read_bind(m_albedoRead)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind(m_normalRead)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind(m_pbrRead)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind(m_depthRead, nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind(m_stencilRead, nyan::Renderpass::Read::Type::ImageStencil)},
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

nyan::DeferredRayShadowsLighting::DeferredRayShadowsLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
	nyan::RenderResource::Id albedoRead, nyan::RenderResource::Id normalRead, nyan::RenderResource::Id pbrRead, nyan::RenderResource::Id depthRead,
	nyan::RenderResource::Id stencilRead, nyan::RenderResource::Id diffuseWrite, nyan::RenderResource::Id specularWrite) :
	Renderer(device, registry, renderManager, pass),
	m_pipeline(device, generate_config()),
	m_albedoRead(albedoRead),
	m_normalRead(normalRead),
	m_pbrRead(pbrRead),
	m_depthRead(depthRead),
	m_stencilRead(stencilRead),
	m_diffuseWrite(diffuseWrite),
	m_specularWrite(specularWrite)
{
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_raytracing_pipeline(m_pipeline);

			render(pipelineBind);

		}, false);
}

void nyan::DeferredRayShadowsLighting::render(vulkan::RaytracingPipelineBind& bind)
{
	auto writeBindDiffuse = r_pass.get_write_bind(m_diffuseWrite, nyan::Renderpass::Write::Type::Compute);
	auto writeBindSpecular = r_pass.get_write_bind(m_specularWrite, nyan::Renderpass::Write::Type::Compute);
	assert(writeBindDiffuse != InvalidResourceId);
	assert(writeBindSpecular != InvalidResourceId);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.albedoBinding {r_pass.get_read_bind(m_albedoRead)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind(m_normalRead)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind(m_pbrRead)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind(m_depthRead, nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind(m_stencilRead, nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseImageBinding {writeBindDiffuse},
		.specularImageBinding {writeBindSpecular},
		
	};
	bind.push_constants(constants);
	bind.trace_rays(m_pipeline, 1920, 1080, 1);

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

nyan::LightComposite::LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass
		, nyan::RenderResource::Id diffuseRead, nyan::RenderResource::Id specularRead) :
	Renderer(device, registry, renderManager, pass),
	m_diffuseRead(diffuseRead),
	m_specularRead(specularRead)
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
		.specularBinding {r_pass.get_read_bind(m_specularRead)},
		.specularSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseBinding {r_pass.get_read_bind(m_diffuseRead)},
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
