#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"

#include "entt/entt.hpp"


nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass),
	m_renderDDGIPipeline(create_pipeline()),
	m_rtPipeline(device, generate_config())
{
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	//For now limit adding ddgi volumes to not allow adding any after render graph build
	//This holds until render graph refactor in regards to modification or rebuild is done
	//Also limit to one ddgi volume for now
	ddgiManager.add_ddgi_volume(nyan::DDGIManager::DDGIVolumeParameters{});
	//pass.add_write("DDGI_Rays", nyan::ImageAttachment
	//		{
	//			.format{VK_FORMAT_R16G16B16A16_SFLOAT},
	//			.size {ImageAttachment::Size::Absolute},
	//			.width { 2048},
	//			.height { 2048},
	//			.clearColor{0.f, 0.f, 0.f, 0.f},
	//		}, nyan::Renderpass::Write::Type::Compute);
	//pass.add_write("DDGI_Irradiance", nyan::ImageAttachment
	//	{
	//		.format{VK_FORMAT_B10G11R11_UFLOAT_PACK32},
	//		.clearColor{0.f, 0.f, 0.f, 0.f},
	//	}, nyan::Renderpass::Write::Type::Compute);

	//pass.add_write("DDGI_Depth", nyan::ImageAttachment
	//	{
	//		.format{VK_FORMAT_R16G16B16A16_SFLOAT},
	//		.clearColor{0.f, 0.f, 0.f, 0.f},
	//	}, nyan::Renderpass::Write::Type::Compute);


	pass.add_renderfunction([this](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd.bind_compute_pipeline(m_renderDDGIPipeline);
			for (uint32_t i = 0; i < r_renderManager.get_ddgi_manager().slot_count(); ++i) {
				render_volume(pipelineBind, i);
			}
		}, false);
}

void nyan::DDGIRenderer::render_volume(vulkan::ComputePipelineBind& bind, uint32_t volumeId)
{
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {volumeId},
		//.col {},
		//.col2 {}
	};
	const auto& volume = ddgiManager.get(volumeId);

	bind.push_constants(constants);
	bind.dispatch(1, 1, 1);
	//bind.trace_rays(&m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callableRegion, 1920, 1080, 1);
}

vulkan::PipelineId nyan::DDGIRenderer::create_pipeline()
{
	vulkan::ComputePipelineConfig pipelineConfig{
		.shaderInstance {r_renderManager.get_shader_manager().add_work_group_size_shader_instance("update_DDGI_comp", 64, 1, 1)},
		.pipelineLayout {r_device.get_bindless_pipeline_layout()}
	};
	return r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
}

vulkan::RaytracingPipelineConfig nyan::DDGIRenderer::generate_config()
{

	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI_rgen")},
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

nyan::DDGIVisualizer::DDGIVisualizer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass),
	m_enabled(true)
{
	create_pipeline();
	r_pass.add_renderfunction([](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{

		}, true);
}

void nyan::DDGIVisualizer::visualize_volume(vulkan::GraphicsPipelineBind& bind, uint32_t volumeId)
{
}

void nyan::DDGIVisualizer::create_pipeline()
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

	r_pass.add_pipeline(staticTangentConfig, &m_pipeline);
}