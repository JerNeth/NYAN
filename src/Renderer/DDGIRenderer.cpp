#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Shader.h"
#include "VulkanWrapper/Image.h"


#include "entt/entt.hpp"


nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass),
	m_rtPipeline(device, generate_config())
{
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	ddgiManager.add_write(r_pass.get_id(), nyan::Renderpass::Write::Type::Compute);
	//For now limit adding ddgi volumes to not allow adding any after render graph build
	//This holds until render graph refactor in regards to modification or rebuild is done
	//Also limit to one ddgi volume for now
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


	BorderPipelineConfig irradianceBorderConfig{
		.workSizeX {m_borderSizeX},
		.workSizeY {m_borderSizeY},
		.workSizeZ {1},
		.columns {true},
		.filterIrradiance {true},
	};

	vulkan::PipelineId irradianceColumnBorderPipelineId = create_border_pipeline(irradianceBorderConfig);
	irradianceBorderConfig.columns = false;
	vulkan::PipelineId irradianceRowBorderPipelineId = create_border_pipeline(irradianceBorderConfig);

	BorderPipelineConfig depthBorderConfig{
		.workSizeX {m_borderSizeX},
		.workSizeY {m_borderSizeY},
		.workSizeZ {1},
		.columns {true},
		.filterIrradiance {false},
	};

	vulkan::PipelineId depthColumnBorderPipelineId = create_border_pipeline(depthBorderConfig);
	depthBorderConfig.columns = false;
	vulkan::PipelineId depthRowBorderPipelineId = create_border_pipeline(depthBorderConfig);


	pass.add_renderfunction([this, irradianceColumnBorderPipelineId,
		irradianceRowBorderPipelineId, depthColumnBorderPipelineId,
		depthRowBorderPipelineId] (vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{
			const auto& ddgiManager = r_renderManager.get_ddgi_manager();
			auto& renderGraph = r_renderManager.get_render_graph();
			auto& resource = renderGraph.get_resource(m_renderTarget);
			//TODO handle sparse set
			VkImageMemoryBarrier2 writeBarrier{
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
			std::array barriers{
			VkImageMemoryBarrier2 {
				.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.srcAccessMask{ 0 },
				.dstStageMask{ VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR },
				.dstAccessMask{ 0 },
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
			},
			VkImageMemoryBarrier2{
				.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.srcAccessMask{ VK_ACCESS_2_SHADER_WRITE_BIT },
				.dstStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.dstAccessMask{ VK_ACCESS_2_SHADER_READ_BIT },
				.oldLayout {VK_IMAGE_LAYOUT_GENERAL},
				.newLayout {VK_IMAGE_LAYOUT_GENERAL},
				.srcQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.dstQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.subresourceRange {
					.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT },
					.baseMipLevel {0},
					.levelCount {VK_REMAINING_MIP_LEVELS},
					.baseArrayLayer {0},
					.layerCount {VK_REMAINING_ARRAY_LAYERS},
				}
			},
			VkImageMemoryBarrier2{
				.sType {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.srcAccessMask{ VK_ACCESS_2_SHADER_WRITE_BIT },
				.dstStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
				.dstAccessMask{ VK_ACCESS_2_SHADER_READ_BIT },
				.oldLayout {VK_IMAGE_LAYOUT_GENERAL},
				.newLayout {VK_IMAGE_LAYOUT_GENERAL},
				.srcQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.dstQueueFamilyIndex{VK_QUEUE_FAMILY_IGNORED},
				.subresourceRange {
					.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT },
					.baseMipLevel {0},
					.levelCount {VK_REMAINING_MIP_LEVELS},
					.baseArrayLayer {0},
					.layerCount {VK_REMAINING_ARRAY_LAYERS},
				}
			} };
			float u = m_dist(m_generator);
			float v = m_dist(m_generator);
			float w = m_dist(m_generator);

			PushConstants constants{
				.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
				.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
				.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
				.ddgiBinding {ddgiManager.get_binding()},
				.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
				.ddgiIndex {0},
				.renderTarget {r_pass.get_write_bind(m_renderTarget, nyan::Renderpass::Write::Type::Compute)},
				.col { 0.4f, 0.6f, 0.8f, 1.0f },
				.randomRotation {sqrt(1- u) * sin(Math::pi_2 * v), sqrt(1-u) * cos(Math::pi_2 * v),
									sqrt(u) * sin(Math::pi_2*w), sqrt(u) * cos(Math::pi_2 * w)}
			};
			assert(resource.handle); 	


			 
			for (uint32_t i = 0; i < static_cast<uint32_t>(r_renderManager.get_ddgi_manager().slot_count()); ++i) {
				//auto pipelineBind = cmd.bind_compute_pipeline(m_filterDDGIPipeline);
				constants.ddgiIndex = i;
				const auto& volume = ddgiManager.get(i);
				const auto& parameters = ddgiManager.get_parameters(i);
				if (!parameters.enabled)
					continue;

				auto& irradiance = renderGraph.get_resource(parameters.irradianceResource);
				auto& depth = renderGraph.get_resource(parameters.depthResource);
				barriers[1].image = *irradiance.handle;
				barriers[2].image = *depth.handle;
				
				PipelineConfig irradianceConfig{
					.workSizeX {volume.irradianceProbeSize},
					.workSizeY {volume.irradianceProbeSize},
					.workSizeZ {1},
					.rayCount {volume.raysPerProbe},
					//.probeSize {volume.raysPerProbe},
					.filterIrradiance {true},
				};
				vulkan::PipelineId irradiancePipelineId = create_filter_pipeline(irradianceConfig);

				PipelineConfig depthConfig{
					.workSizeX {volume.depthProbeSize},
					.workSizeY {volume.depthProbeSize},
					.workSizeZ {1},
					.rayCount {volume.raysPerProbe},
					//.probeSize {volume.raysPerProbe},
					.filterIrradiance {false},
				};
				vulkan::PipelineId depthPipelineId = create_filter_pipeline(depthConfig);
				
				auto rtPipelineBind = cmd.bind_raytracing_pipeline(m_rtPipeline);
				render_volume(rtPipelineBind, constants, volume.raysPerProbe, volume.probeCountX * volume.probeCountY * volume.probeCountZ);
				cmd.barrier2(1, &writeBarrier);

				auto pipelineBind = cmd.bind_compute_pipeline(irradiancePipelineId);
				filter_volume(pipelineBind, constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
				pipelineBind = cmd.bind_compute_pipeline(depthPipelineId);
				filter_volume(pipelineBind, constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
				cmd.barrier2(barriers.size(), barriers.data());

				pipelineBind = cmd.bind_compute_pipeline(irradianceColumnBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.probeCountX, volume.irradianceTextureSizeY, 1);
				pipelineBind = cmd.bind_compute_pipeline(irradianceRowBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.irradianceTextureSizeX, volume.probeCountY* volume.probeCountZ, 1);
				pipelineBind = cmd.bind_compute_pipeline(depthColumnBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.probeCountX, volume.depthTextureSizeY, 1);
				pipelineBind = cmd.bind_compute_pipeline(depthRowBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.depthTextureSizeX, volume.probeCountY* volume.probeCountZ, 1);


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
	}
	else {
		auto& resource = renderGraph.get_resource(m_renderTarget);
		auto& image = std::get<nyan::ImageAttachment>(resource.attachment);
		image.width = static_cast<float>(maxRays);
		image.height = static_cast<float>(maxProbes);

	}
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const PushConstants& constants, uint32_t numRays, uint32_t numProbes)
{
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);

	//const auto& volume = ddgiManager.get(volumeId);

	bind.push_constants(constants);
	
	bind.trace_rays(m_rtPipeline, numRays, numProbes, 1);
}

void nyan::DDGIRenderer::filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ)
{
	bind.push_constants(constants);
	bind.dispatch(probeCountX, probeCountY, probeCountZ);
}

void nyan::DDGIRenderer::copy_borders(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ)
{
	bind.push_constants(constants);
	bind.dispatch(static_cast<uint32_t>(std::ceil(probeCountX / static_cast<float>(m_borderSizeX))),
		static_cast<uint32_t>(std::ceil(probeCountY / static_cast<float>(m_borderSizeY))),
		probeCountZ);
}

vulkan::PipelineId nyan::DDGIRenderer::create_filter_pipeline(const PipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_pipelines.find(config); it != m_pipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
			.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("update_DDGI_comp",
			config.workSizeX, config.workSizeY,
			config.workSizeZ, vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::rayCountShaderName, config.rayCount}
			//, vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::probeSizeShaderName, config.probeSize}
			, vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::filterIrradianceShaderName, config.filterIrradiance})},
			.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_pipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}

vulkan::PipelineId nyan::DDGIRenderer::create_border_pipeline(const BorderPipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_borderPipelines.find(config); it != m_borderPipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("update_DDGI_borders_comp",
				config.workSizeX, config.workSizeY,
				config.workSizeZ
				// ,vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::rayCountShaderName, config.rayCount},
				,vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::columnsShaderName, config.columns}
				,vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::filterIrradianceShaderName, config.filterIrradiance}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_borderPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
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
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI_rchit")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI_rmiss")},
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

nyan::DDGIVisualizer::DDGIVisualizer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting, const nyan::RenderResource::Id& depth) :
	Renderer(device, registry, renderManager, pass),
	m_enabled(true),
	m_lighting(lighting),
	m_depth(depth)
{
	r_pass.add_depth_attachment(m_depth);
	r_pass.add_attachment(m_lighting.specular);
	r_pass.add_attachment(m_lighting.diffuse);
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	ddgiManager.add_read(r_pass.get_id());
	create_pipeline();
	r_pass.add_renderfunction([this, &ddgiManager](vulkan::CommandBuffer& cmd, nyan::Renderpass&)
		{

			auto pipelineBind = cmd.bind_graphics_pipeline(m_pipeline);
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
			for (uint32_t volumeId{ 0 }; volumeId < ddgiManager.slot_count(); volumeId++) {
				if (ddgiManager.get_parameters(volumeId).visualization) {
					visualize_volume(pipelineBind, volumeId);
				}
			}
		}, true);
}

void nyan::DDGIVisualizer::visualize_volume(vulkan::GraphicsPipelineBind& bind, uint32_t volumeId)
{
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	const auto& volume = ddgiManager.get(volumeId);
	PushConstants pushConstants{
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {volumeId}
	};
	bind.push_constants(pushConstants);
	bind.draw(volume.probeCountX * volume.probeCountY * volume.probeCountZ * 6, 1);
}

void nyan::DDGIVisualizer::create_pipeline()
{

	vulkan::GraphicsPipelineConfig visualizerConfig{
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
	.vertexInputCount = 0,
	.shaderCount = 2,
	.vertexInputFormats {
	},
	.shaderInstances {
		r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_visualizer_vert"),
		r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_visualizer_frag")
	},
	.pipelineLayout = r_device.get_bindless_pipeline_layout(),
	};
	visualizerConfig.dynamicState.depth_write_enable = VK_TRUE;
	visualizerConfig.dynamicState.depth_test_enable = VK_TRUE;
	visualizerConfig.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	visualizerConfig.dynamicState.stencil_test_enable = VK_FALSE;

	r_pass.add_pipeline(visualizerConfig, &m_pipeline);
}