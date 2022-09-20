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

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);

			
			render(pipelineBind);
		}, true);
}

void nyan::DeferredLighting::render(vulkan::GraphicsPipelineBind& bind)
{
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
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
		.ddgiBinding{ ddgiManager.get_binding() },
		.ddgiCount{ static_cast<uint32_t>(ddgiManager.slot_count()) },
		.ddgiIndex{ 0 },
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
	const GBuffer& gBuffer, const Lighting& lighting) :
	Renderer(device, registry, renderManager, pass),
	m_pipeline(device, generate_config()),
	m_gbuffer(gBuffer),
	m_lighting(lighting)
{
	r_pass.add_read(m_gbuffer.albedo);
	r_pass.add_read(m_gbuffer.normal);
	r_pass.add_read(m_gbuffer.pbr);
	r_pass.add_read(m_gbuffer.depth, nyan::Renderpass::Read::Type::ImageDepth);
	r_pass.add_read(m_gbuffer.stencil, nyan::Renderpass::Read::Type::ImageStencil);
	r_pass.add_write(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	r_pass.add_write(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	renderManager.get_ddgi_manager().add_read(pass.get_id());
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_raytracing_pipeline(m_pipeline);

			render(pipelineBind);

		}, false);
}

void nyan::DeferredRayShadowsLighting::render(vulkan::RaytracingPipelineBind& bind)
{
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
	auto writeBindDiffuse = r_pass.get_write_bind(m_lighting.diffuse, nyan::Renderpass::Write::Type::Compute);
	auto writeBindSpecular = r_pass.get_write_bind(m_lighting.specular, nyan::Renderpass::Write::Type::Compute);
	assert(writeBindDiffuse != InvalidBinding);
	assert(writeBindSpecular != InvalidBinding);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {0},
		.albedoBinding {r_pass.get_read_bind(m_gbuffer.albedo)},
		.albedoSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.normalBinding {r_pass.get_read_bind(m_gbuffer.normal)},
		.normalSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.pbrBinding {r_pass.get_read_bind(m_gbuffer.pbr)},
		.pbrSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.depthBinding {r_pass.get_read_bind(m_gbuffer.depth, nyan::Renderpass::Read::Type::ImageDepth)},
		.depthSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.stencilBinding {r_pass.get_read_bind(m_gbuffer.stencil, nyan::Renderpass::Read::Type::ImageStencil)},
		.stencilSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseImageBinding {writeBindDiffuse},
		.specularImageBinding {writeBindSpecular},
		
	};
	const auto& writeSpecular = r_pass.get_graph().get_resource(m_lighting.specular);
	const auto& attachment = std::get<ImageAttachment>(writeSpecular.attachment);
	uint32_t width {0}, height {0};
	if (attachment.size == ImageAttachment::Size::Swapchain) {
		width = attachment.width * r_device.get_swapchain_width();
		height = attachment.height * r_device.get_swapchain_height();
	}
	else {
		width = static_cast<uint32_t>(attachment.width);
		height = static_cast<uint32_t>(attachment.height);
	}
	bind.push_constants(constants);
	bind.trace_rays(m_pipeline, width, height, 1);

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
		.hitGroups {
			vulkan::Group
			{
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("transparent_rahit")},
			},
			vulkan::Group
			{
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("transparent_rahit")},
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
		, const Lighting& lighting) :
	Renderer(device, registry, renderManager, pass),
	m_lighting(lighting)
{
	r_pass.add_read(m_lighting.diffuse);
	r_pass.add_read(m_lighting.specular);
	r_pass.add_swapchain_attachment(Math::vec4{}, true);
	create_pipeline();
	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_graphics_pipeline(m_compositePipeline);

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);


			render(pipelineBind);
		}, true);
}

void nyan::LightComposite::render(vulkan::GraphicsPipelineBind& bind)
{
	PushConstants constants{
		.specularBinding {r_pass.get_read_bind(m_lighting.specular)},
		.specularSampler {static_cast<uint32_t>(vulkan::DefaultSampler::NearestClamp)},
		.diffuseBinding {r_pass.get_read_bind(m_lighting.diffuse)},
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
