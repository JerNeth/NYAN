#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"

#include "entt/entt.hpp"


nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass),
	m_filterDDGIPipeline(create_pipeline()),
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
			const auto& ddgiManager = r_renderManager.get_ddgi_manager();
			auto& renderGraph = r_renderManager.get_render_graph();
			auto& resource = renderGraph.get_resource(m_renderTarget);
			//TODO handle sparse set
			VkImageMemoryBarrier2 readBarrier{
				.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask{ VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR },
				.srcAccessMask{ VK_ACCESS_2_SHADER_WRITE_BIT },
				.dstStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.dstAccessMask{ VK_ACCESS_2_SHADER_READ_BIT },
				.oldLayout {VK_IMAGE_LAYOUT_GENERAL},
				.newLayout {VK_IMAGE_LAYOUT_GENERAL},
				.srcQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.dstQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.image {*resource.handle},
				.subresourceRange {
					.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT },
					.baseMipLevel {0},
					.levelCount {VK_REMAINING_MIP_LEVELS},
					.baseArrayLayer {0},
					.layerCount {VK_REMAINING_ARRAY_LAYERS},
				}
			};
			VkImageMemoryBarrier2 writeBarrier{
				.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.srcAccessMask{ 0 },
				.dstStageMask{ VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR },
				.dstAccessMask{ 0  },
				.oldLayout {VK_IMAGE_LAYOUT_GENERAL},
				.newLayout {VK_IMAGE_LAYOUT_GENERAL},
				.srcQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.dstQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.image {*resource.handle},
				.subresourceRange {
					.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT },
					.baseMipLevel {0},
					.levelCount {VK_REMAINING_MIP_LEVELS},
					.baseArrayLayer {0},
					.layerCount {VK_REMAINING_ARRAY_LAYERS},
				}
			};
			PushConstants constants{
				.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
				.ddgiBinding {ddgiManager.get_binding()},
				.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
				.ddgiIndex {0},
				.renderTarget {r_pass.get_write_bind(m_renderTarget)}
				//.col {},
				//.col2 {}
			};
			assert(resource.handle); 
			for (uint32_t i = 0; i < r_renderManager.get_ddgi_manager().slot_count(); ++i) {

				//auto pipelineBind = cmd.bind_compute_pipeline(m_filterDDGIPipeline);
				constants.ddgiIndex = i;
				auto rtPipelineBind = cmd.bind_raytracing_pipeline(m_rtPipeline);
				render_volume(rtPipelineBind, constants);
				cmd.barrier2(0, 0, nullptr, 0, nullptr, 1, &readBarrier);
				auto filterRtPipelineBind = cmd.bind_compute_pipeline(m_filterDDGIPipeline);
				cmd.barrier2(0, 0, nullptr, 0, nullptr, 1, &writeBarrier);
				filter_volume(filterRtPipelineBind, constants);
			}
		}, false);
}

void nyan::DDGIRenderer::begin_frame() 
{
	uint32_t maxRays = 0;
	uint32_t maxProbes = 0;
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	for (uint32_t i = 0; i < ddgiManager.slot_count(); ++i) {
		const auto& volume = ddgiManager.get(i);
		auto rayCount =  volume.raysPerProbe;
		auto probeCount = volume.probeCountX * volume.probeCountY * volume.probeCountZ;
		if (rayCount > maxRays)
			maxRays = rayCount;
		if (probeCount > maxProbes)
			maxProbes = probeCount;
	}
	auto& renderGraph = r_renderManager.get_render_graph();
	if (!m_renderTarget) {
		m_renderTarget = renderGraph.add_ressource("DDGI_RenderTarget", nyan::ImageAttachment
			{
				.format{VK_FORMAT_R16G16B16A16_SFLOAT},
				.size {ImageAttachment::Size::Absolute},
				.width { static_cast<float>(maxRays)},
				.height { static_cast<float>(maxProbes)},
				.clearColor{0.f, 0.f, 0.f, 0.f},
			});
		r_pass.add_write(m_renderTarget, nyan::Renderpass::Write::Type::Compute);
		ddgiManager.add_write(r_pass.get_id(), nyan::Renderpass::Write::Type::Compute);
	}
	else {
		auto& resource = renderGraph.get_resource(m_renderTarget);
		auto& image = std::get<nyan::ImageAttachment>(resource.attachment);
		image.width = static_cast<float>(maxRays);
		image.height = static_cast<float>(maxProbes);

	}
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const PushConstants& constants)
{
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);

	//const auto& volume = ddgiManager.get(volumeId);

	bind.push_constants(constants);
	
	bind.trace_rays(m_rtPipeline, /*numRays*/1, /*numProbes*/1, 1);
}

void nyan::DDGIRenderer::filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants)
{
	bind.push_constants(constants);
	//bind.dispatch(1, 1, 1);
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