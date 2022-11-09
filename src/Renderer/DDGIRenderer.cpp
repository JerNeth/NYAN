#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Shader.h"
#include "VulkanWrapper/Image.h"


#include "entt/entt.hpp"


nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass)
{
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	ddgiManager.add_write(r_pass.get_id(), nyan::Renderpass::Write::Type::Compute);

	pass.add_renderfunction([this] (vulkan::CommandBuffer& cmd, nyan::Renderpass&)
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
			VkMemoryBarrier2 global{
				.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
				.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
				.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
				.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
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
					.filterIrradiance {true},
					.renderTargetImageFormat {volume.renderTargetImageFormat},
					.imageFormat {volume.irradianceImageFormat},
					.renderTargetImageWidthBits{m_renderTargetWidthBits},
				};
				vulkan::PipelineId irradiancePipelineId = create_filter_pipeline(irradianceConfig);

				PipelineConfig depthConfig{
					.workSizeX {volume.depthProbeSize},
					.workSizeY {volume.depthProbeSize},
					.workSizeZ {1},
					.rayCount {volume.raysPerProbe},
					.filterIrradiance {false},
					.renderTargetImageFormat {volume.renderTargetImageFormat},
					.imageFormat {volume.depthImageFormat},
					.renderTargetImageWidthBits{m_renderTargetWidthBits},
				};
				vulkan::PipelineId depthPipelineId = create_filter_pipeline(depthConfig);

				BorderPipelineConfig irradianceBorderConfig{
					.workSizeX {m_borderSizeX},
					.workSizeY {m_borderSizeY},
					.workSizeZ {1},
					.columns {true},
					.filterIrradiance {true},
					.imageFormat{volume.irradianceImageFormat}
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
					.imageFormat{volume.depthImageFormat}
				};

				vulkan::PipelineId depthColumnBorderPipelineId = create_border_pipeline(depthBorderConfig);
				depthBorderConfig.columns = false;
				vulkan::PipelineId depthRowBorderPipelineId = create_border_pipeline(depthBorderConfig);


				RelocatePipelineConfig relocateConfig{
					.workSizeX {32},
					.workSizeY {1},
					.workSizeZ {1},
					.relocationEnabled {VK_FALSE},
					.renderTargetImageWidthBits {m_renderTargetWidthBits},
					.renderTargetImageFormat {volume.renderTargetImageFormat},
				};
				vulkan::PipelineId relocatePipelineDisabledId = create_relocate_pipeline(relocateConfig);
				relocateConfig.relocationEnabled = VK_TRUE;
				vulkan::PipelineId relocatePipelineEnabledId = create_relocate_pipeline(relocateConfig);

				RTConfig rtConfig{
					.renderTargetImageWidthBits {m_renderTargetWidthBits},
					.renderTargetImageFormat {volume.renderTargetImageFormat},
					.numRows{ parameters.numRowsRaygen}
				};
				//calculate numRaysPerProbe
				//bind scanPipeline
				//do prefixSum (exclusive) of numRaysPerProbe
				//calculate total num rays
				PushConstantsDynamic dynamicConstants{
					.ddgiBinding {constants.ddgiBinding},
					.ddgiCount  {constants.ddgiCount},
					.ddgiIndex {constants.ddgiIndex},
					.numRows {rtConfig.numRows}
				};
				auto& rtPipeline = get_rt_pipeline(rtConfig);
				//if (volume.dynamicRayAllocationEnabled) {
				if (parameters.dynamicRayAllocation) {
					assert(m_scratchBufferBinding != ~0);
					dynamicConstants.workBufferBinding = m_scratchBufferBinding;
					dynamicConstants.targetAddress = ddgiManager.get_ray_count_device_address(i);
					//PrefixSumPipelineConfig prefixSumPipelineConfig{
					//	.workSizeX {m_prefixSumGroupSize},
					//	.workSizeY {1},
					//	.workSizeZ {1},
					//	.subgroupSize {r_device.get_physical_device().get_subgroup_properties().subgroupSize},
					//	.numRows {m_prefixSumNumRows},
					//	.renderTargetImageWidthBits {m_renderTargetWidthBits},
					//};
					//auto prefixSumPipeline = create_prefix_sum_pipeline(prefixSumPipelineConfig);
					//auto pipelineBind = cmd.bind_compute_pipeline(prefixSumPipeline);
					//prefix_sum(pipelineBind, dynamicConstants, (volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((m_prefixSumGroupSize * m_prefixSumNumRows) - 1)) / (m_prefixSumGroupSize * m_prefixSumNumRows), 1, 1);
					//VkMemoryBarrier2 globalRay{
					//	.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
					//	.pNext {nullptr},
					//	.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
					//	.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
					//	.dstStageMask {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
					//	.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT}, //VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT Does not work with indirect traces
					//};
					//cmd.barrier2(1, &globalRay);
					auto rtPipelineBind = cmd.bind_raytracing_pipeline(rtPipeline);
					render_volume(rtPipelineBind, rtPipeline, constants, dynamicConstants.targetAddress);
				}
				else {
					auto rtPipelineBind = cmd.bind_raytracing_pipeline(rtPipeline);
					render_volume(rtPipelineBind, rtPipeline, constants, (1ul << m_renderTargetWidthBits) / rtConfig.numRows,
						(volume.raysPerProbe * volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((1 << m_renderTargetWidthBits)+ 1)) >> m_renderTargetWidthBits);
				}
				cmd.barrier2(1, &writeBarrier);

				auto pipelineBind = cmd.bind_compute_pipeline(irradiancePipelineId);
				filter_volume(pipelineBind, constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
				pipelineBind = cmd.bind_compute_pipeline(depthPipelineId);
				filter_volume(pipelineBind, constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
				cmd.barrier2(1, &global, 0, nullptr, static_cast<uint32_t>(barriers.size()), barriers.data());

				ScanPipelineConfig scanPipelineCfg{
					.workSizeX {m_scanGroupSize},
					.workSizeY {1},
					.workSizeZ {1},
					.subgroupSize {r_device.get_physical_device().get_subgroup_properties().subgroupSize},
					.numRows {m_scanNumRows}
				};

				if (volume.dynamicRayAllocationEnabled && false) {
					auto scanPipeline = create_scan_pipeline(scanPipelineCfg);
					pipelineBind = cmd.bind_compute_pipeline(scanPipeline);
					sum_variance(pipelineBind, dynamicConstants, (volume.probeCountX* volume.probeCountY* volume.probeCountZ + ((m_scanGroupSize  * m_scanNumRows)- 1)) / (m_scanGroupSize * m_scanNumRows), 1, 1);

				}
				


				if (parameters.frames <= 1) {
					pipelineBind = cmd.bind_compute_pipeline(relocatePipelineDisabledId);

					relocate_probes(pipelineBind, constants,
						(volume.probeCountX* volume.probeCountY* volume.probeCountZ + (relocateConfig.workSizeX - 1))
						/ relocateConfig.workSizeX, 1, 1);
				}
				else if (volume.relocationEnabled) {
					pipelineBind = cmd.bind_compute_pipeline(relocatePipelineEnabledId);

					relocate_probes(pipelineBind, constants,
						(volume.probeCountX * volume.probeCountY * volume.probeCountZ + (relocateConfig.workSizeX - 1))
						/ relocateConfig.workSizeX, 1, 1);
				}

				pipelineBind = cmd.bind_compute_pipeline(irradianceColumnBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.probeCountX, volume.irradianceTextureSizeY, volume.probeCountZ);
				pipelineBind = cmd.bind_compute_pipeline(irradianceRowBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.irradianceTextureSizeX, volume.probeCountY, volume.probeCountZ);
				pipelineBind = cmd.bind_compute_pipeline(depthColumnBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.probeCountX, volume.depthTextureSizeY, volume.probeCountZ);
				pipelineBind = cmd.bind_compute_pipeline(depthRowBorderPipelineId);
				copy_borders(pipelineBind, constants, volume.depthTextureSizeX, volume.probeCountY, volume.probeCountZ);

				if (volume.dynamicRayAllocationEnabled && false) {
					GatherPipelineConfig gatherPipelineCfg{
						.workSizeX {m_gatherGroupSize},
						.workSizeY {1},
						.workSizeZ {1},
						.numRows {m_gatherNumRows}
					};
					auto gatherPipeline = create_gather_pipeline(gatherPipelineCfg);
					pipelineBind = cmd.bind_compute_pipeline(gatherPipeline);
					cmd.barrier2(1, &global);
					gather_variance(pipelineBind, dynamicConstants, (volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((m_gatherGroupSize * m_gatherNumRows )- 1)) / (m_gatherGroupSize * m_gatherNumRows), 1, 1);
				}

			}
		}, false);
}

void nyan::DDGIRenderer::begin_frame() 
{
	uint32_t maxRays = 0;
	uint32_t maxScratchSize = 0;
	VkFormat renderTargetFormat{ VK_FORMAT_UNDEFINED };
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	for (uint32_t i = 0; i < ddgiManager.slot_count(); ++i) {
		const auto& volume = ddgiManager.get(i);
		auto probeCount = volume.probeCountX * volume.probeCountY * volume.probeCountZ;
		auto rayCount = probeCount * volume.raysPerProbe;
		if (rayCount > maxRays)
			maxRays = rayCount;
		auto scratchSize = (probeCount + (m_scanGroupSize -1 ))/ m_scanGroupSize;
		if (scratchSize > maxScratchSize && volume.dynamicRayAllocationEnabled)
			maxScratchSize = scratchSize;

		switch (volume.renderTargetImageFormat) {
		case nyan::shaders::R32G32B32A32F:
			renderTargetFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		default:
			assert(false);
			[[fallthrough]];
		case nyan::shaders::R16G16B16A16F:
			renderTargetFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
			break;
		}
	}

	auto& renderGraph = r_renderManager.get_render_graph();
	auto width = static_cast<float>(1 << m_renderTargetWidthBits);
	auto height = std::ceil(static_cast<float>(maxRays) / static_cast<float>(1 << m_renderTargetWidthBits));
	if (!m_renderTarget) {
		m_renderTarget = renderGraph.add_ressource("DDGI_RenderTarget", nyan::ImageAttachment
			{
				.format{renderTargetFormat},//VK_FORMAT_R16G16B16A16_SFLOAT},
				.size {ImageAttachment::Size::Absolute},
				.width { width},
				.height { height},
				.clearColor{0.f, 0.f, 0.f, 0.f},
			});
		r_pass.add_write(m_renderTarget, nyan::Renderpass::Write::Type::Compute);
	}
	else {
		auto& resource = renderGraph.get_resource(m_renderTarget);
		auto& image = std::get<nyan::ImageAttachment>(resource.attachment);
		image.width = width;
		image.height = height;
		image.format = renderTargetFormat;

	}
	if (maxScratchSize > 0 && !m_scratchBuffer) {
		vulkan::BufferInfo info{
			.size {maxScratchSize * sizeof(uint32_t) * 4},
			.usage {VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT },
			.memoryUsage {VMA_MEMORY_USAGE_GPU_ONLY},
		};
		m_scratchBuffer = std::make_unique<vulkan::BufferHandle>(r_device.create_buffer(info, {}, false));
		m_scratchBufferBinding = r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{(*m_scratchBuffer)->get_handle(), 0, info.size});
		
	}
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const PushConstants& constants, uint32_t numRays, uint32_t numProbes)
{
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);

	//const auto& volume = ddgiManager.get(volumeId);

	bind.push_constants(constants);

	assert(numRays * numRays <= r_device.get_physical_device().get_ray_tracing_pipeline_properties().maxRayDispatchInvocationCount);
	[[maybe_unused]] const auto* const groupsSize = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize;
	[[maybe_unused]] const auto* const groupsCount = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(numRays <= groupsSize[0] * groupsCount[0]);
	assert(numProbes <= groupsSize[1] * groupsCount[1]);
	bind.trace_rays(pipeline, numRays, numProbes, 1);
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const PushConstants& constants, VkDeviceAddress address)
{
	bind.push_constants(constants);

	bind.trace_rays(pipeline, address);
}

void nyan::DDGIRenderer::filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ)
{
	bind.push_constants(constants);
	[[maybe_unused]] const auto* const groupsCount = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(probeCountX <= groupsCount[0]);
	assert(probeCountY <= groupsCount[1]);
	assert(probeCountZ <= groupsCount[2]);
	bind.dispatch(probeCountX, probeCountY, probeCountZ);
}

void nyan::DDGIRenderer::copy_borders(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ)
{
	bind.push_constants(constants);
	auto groupCountX = static_cast<uint32_t>(std::ceil(probeCountX / static_cast<float>(m_borderSizeX)));
	auto groupCountY = static_cast<uint32_t>(std::ceil(probeCountY / static_cast<float>(m_borderSizeY)));
	auto groupCountZ = probeCountZ;
	[[maybe_unused]] const auto* const groupCountLimit = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(groupCountX <= groupCountLimit[0]);
	assert(groupCountY <= groupCountLimit[1]);
	assert(groupCountZ <= groupCountLimit[2]);
	bind.dispatch(groupCountX, groupCountY, groupCountZ);
}

void nyan::DDGIRenderer::relocate_probes(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	bind.push_constants(constants);
	const auto* const groupCountLimit = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(groupCountX <= groupCountLimit[0]);
	assert(groupCountY <= groupCountLimit[1]);
	assert(groupCountZ <= groupCountLimit[2]);
	bind.dispatch(groupCountX, groupCountY, groupCountZ);
}

void nyan::DDGIRenderer::sum_variance(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	bind.push_constants(constants);
	const auto* const groupCountLimit = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(groupCountX <= groupCountLimit[0]);
	assert(groupCountY <= groupCountLimit[1]);
	assert(groupCountZ <= groupCountLimit[2]);
	bind.dispatch(groupCountX, groupCountY, groupCountZ);
}

void nyan::DDGIRenderer::gather_variance(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	bind.push_constants(constants);
	const auto* const groupCountLimit = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(groupCountX <= groupCountLimit[0]);
	assert(groupCountY <= groupCountLimit[1]);
	assert(groupCountZ <= groupCountLimit[2]);
	bind.dispatch(groupCountX, groupCountY, groupCountZ);
}

void nyan::DDGIRenderer::prefix_sum(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ)
{
	bind.push_constants(constants);
	const auto* const groupCountLimit = r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount;
	assert(groupCountX <= groupCountLimit[0]);
	assert(groupCountY <= groupCountLimit[1]);
	assert(groupCountZ <= groupCountLimit[2]);
	bind.dispatch(groupCountX, groupCountY, groupCountZ);
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
			config.workSizeZ, vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::rayCountShaderName, config.rayCount},
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::filterIrradianceShaderName, config.filterIrradiance},
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::renderTargetImageFormatShaderName, config.renderTargetImageFormat},
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::imageFormatShaderName, config.imageFormat },
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits })
			},
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
				config.workSizeZ,
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::columnsShaderName, config.columns},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::filterIrradianceShaderName, config.filterIrradiance},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::imageFormatShaderName, config.imageFormat}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_borderPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}

vulkan::PipelineId nyan::DDGIRenderer::create_relocate_pipeline(const RelocatePipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_relocatePipelines.find(config); it != m_relocatePipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_relocate_comp",
				config.workSizeX, config.workSizeY,
				config.workSizeZ,
				vulkan::ShaderStorage::SpecializationConstant{RelocatePipelineConfig::relocationEnabledShaderName, config.relocationEnabled},
				vulkan::ShaderStorage::SpecializationConstant{RelocatePipelineConfig::renderTargetImageFormatShaderName, config.renderTargetImageFormat},
				vulkan::ShaderStorage::SpecializationConstant{RelocatePipelineConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_relocatePipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}
vulkan::PipelineId nyan::DDGIRenderer::create_scan_pipeline(const ScanPipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_scanPipelines.find(config); it != m_scanPipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_sum_comp",
				config.workSizeX, config.workSizeY,
				config.workSizeZ,
				vulkan::ShaderStorage::SpecializationConstant{ScanPipelineConfig::subgroupSizeShaderName, config.subgroupSize},
				vulkan::ShaderStorage::SpecializationConstant{ScanPipelineConfig::numRowsShaderName, config.numRows}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_scanPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}
vulkan::PipelineId nyan::DDGIRenderer::create_gather_pipeline(const GatherPipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_gatherPipelines.find(config); it != m_gatherPipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_gather_comp",
				config.workSizeX, config.workSizeY,
				config.workSizeZ,
				vulkan::ShaderStorage::SpecializationConstant{GatherPipelineConfig::numRowsShaderName, config.numRows}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_gatherPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}
vulkan::PipelineId nyan::DDGIRenderer::create_prefix_sum_pipeline(const PrefixSumPipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_prefixSumPipelines.find(config); it != m_prefixSumPipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_prefix_sum_comp",
				config.workSizeX, config.workSizeY,
				config.workSizeZ,
				vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::subgroupSizeShaderName, config.subgroupSize},
				vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::numRowsShaderName, config.numRows},
				vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_prefixSumPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}
vulkan::RTPipeline& nyan::DDGIRenderer::get_rt_pipeline(const RTConfig& config)
{
	if (auto it = m_rtPipelines.find(config); it != m_rtPipelines.end()) {
		assert(it->second);
		return *it->second;
	}
	else {
		return *m_rtPipelines.emplace(config, std::make_unique< vulkan::RTPipeline>(r_device, generate_config(config))).first->second;
	}
}

vulkan::RaytracingPipelineConfig nyan::DDGIRenderer::generate_config(const RTConfig& config)
{

	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI_rgen"
				,vulkan::ShaderStorage::SpecializationConstant{RTConfig::renderTargetImageFormatShaderName, config.renderTargetImageFormat}
				,vulkan::ShaderStorage::SpecializationConstant{RTConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits}
				,vulkan::ShaderStorage::SpecializationConstant{RTConfig::numRowsShaderName, config.numRows})},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_DDGI_rchit")},
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_alpha_test_rahit")},
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

			auto [viewport, scissor] = r_device.get_swapchain_viewport_and_scissor();
			pipelineBind.set_scissor_with_count(1, &scissor);
			pipelineBind.set_viewport_with_count(1, &viewport);
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
	float u = m_dist(m_generator);
	float v = m_dist(m_generator);
	float w = m_dist(m_generator);
	PushConstants pushConstants{
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {volumeId},
		.randomRotation {sqrt(1 - u) * sin(Math::pi_2 * v), sqrt(1 - u) * cos(Math::pi_2 * v),
						sqrt(u) * sin(Math::pi_2 * w), sqrt(u) * cos(Math::pi_2 * w)}
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