#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"
#include "VulkanWrapper/Shader.h"
#include "VulkanWrapper/Image.h"
#include "VulkanWrapper/PhysicalDevice.hpp"

#include "stb_image_write.h"

#include "entt/entt.hpp"


nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass)
{
	auto& ddgiManager = r_renderManager.get_ddgi_manager();
	ddgiManager.add_write(r_pass.get_id(), nyan::Renderpass::Write::Type::Compute);
	
	//vulkan::BufferInfo info{
	//	.size {sizeof(VkTraceRaysIndirectCommandKHR)},
	//	.usage {VK_BUFFER_USAGE_TRANSFER_DST_BIT },
	//	.memoryUsage {VMA_MEMORY_USAGE_CPU_ONLY},
	//};
	//dstBuf = std::make_unique< vulkan::BufferHandle>(r_device.create_buffer(info, {}));
	//auto probecount = (24 * 22 * 24 + 1);
	//auto uintPart = probecount * 2;
	//auto floatPart = probecount * 3 + 1;
	//info.size = uintPart * sizeof(uint32_t) + sizeof(float) * floatPart;
	//dstBuf2 = std::make_unique< vulkan::BufferHandle>(r_device.create_buffer(info, {}));

	pass.add_renderfunction(std::bind(&DDGIRenderer::render, this, std::placeholders::_1, std::placeholders::_2), false);
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
			.usage {VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT },
			.memoryUsage {VMA_MEMORY_USAGE_GPU_ONLY},
		};
		std::vector<uint32_t> data(maxScratchSize * 4, 0);

		m_scratchBuffer = std::make_unique<vulkan::BufferHandle>(r_device.create_buffer(info, { vulkan::InputData{.ptr {data.data()}, .size {info.size}} }, false));
		m_scratchBufferBinding = r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{(*m_scratchBuffer)->get_handle(), 0, info.size});
		
	}
}

void nyan::DDGIRenderer::render(vulkan::CommandBuffer& cmd, nyan::Renderpass&)
{
	if (!m_enabled)
		return;
	const auto& ddgiManager = r_renderManager.get_ddgi_manager();
	//TODO handle sparse set
	static constexpr VkMemoryBarrier2 writeBarrier{
		.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask{ VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR },
		.srcAccessMask{ VK_ACCESS_2_SHADER_WRITE_BIT },
		.dstStageMask{ VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT },
		.dstAccessMask{ VK_ACCESS_2_SHADER_READ_BIT }
	};
	float u = m_dist(*m_generator);
	float v = m_dist(*m_generator);
	float w = m_dist(*m_generator);

	DDGIPushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.ddgiBinding {ddgiManager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgiManager.slot_count())},
		.ddgiIndex {0},
		.renderTarget {r_pass.get_write_bind(m_renderTarget, nyan::Renderpass::Write::Type::Compute)},
		.randomRotation {sqrt(1 - u) * sin(Math::pi_2 * v), sqrt(1 - u) * cos(Math::pi_2 * v),
							sqrt(u) * sin(Math::pi_2 * w), sqrt(u) * cos(Math::pi_2 * w)}
	};


	for (uint32_t i = 0; i < static_cast<uint32_t>(r_renderManager.get_ddgi_manager().slot_count()); ++i) {
		//auto pipelineBind = cmd.bind_compute_pipeline(m_filterDDGIPipeline);
		constants.ddgiIndex = i;
		const auto& volume = ddgiManager.get(i);
		const auto& parameters = ddgiManager.get_parameters(i);
		if (!parameters.enabled)
			continue;

		RTConfig rtConfig{
			.renderTargetImageWidthBits {m_renderTargetWidthBits},
			.renderTargetImageFormat {volume.renderTargetImageFormat},
			.numRows{ parameters.numRowsRaygen}, //Deprecated
			.dynamicRayAllocationEnabled {volume.dynamicRayAllocationEnabled}
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
		if (volume.dynamicRayAllocationEnabled) {
			r_renderManager.get_profiler().begin_profile(cmd, "Dynamic Ray Allocation");
			//if (parameters.dynamicRayAllocation) {
			assert(m_scratchBufferBinding != ~0);
			dynamicConstants.workBufferBinding = m_scratchBufferBinding;
			dynamicConstants.targetAddress = ddgiManager.get_ray_count_device_address(i);
			PrefixSumPipelineConfig prefixSumPipelineConfig{
				.workSizeX {m_prefixSumGroupSize},
				.workSizeY {1},
				.workSizeZ {1},
				.subgroupSize {r_device.get_physical_device().get_subgroup_properties().subgroupSize},
				.numRows {m_prefixSumNumRows},
				.renderTargetImageWidthBits {m_renderTargetWidthBits},
			};
			auto prefixSumPipeline = create_prefix_sum_pipeline(prefixSumPipelineConfig);
			auto pipelineBind = cmd.bind_compute_pipeline(prefixSumPipeline);
			prefix_sum(pipelineBind, dynamicConstants, (volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((m_prefixSumGroupSize * m_prefixSumNumRows) - 1)) / (m_prefixSumGroupSize * m_prefixSumNumRows), 1, 1);
			VkMemoryBarrier2 globalRay{
				.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
				.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
				.dstStageMask {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
				.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT}, //VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT Does not work with indirect traces
			};
			//VkMemoryBarrier2 globalRay{
			//	.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
			//	.pNext {nullptr},
			//	.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
			//	.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
			//	.dstStageMask {VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT},
			//	.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT}, //VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT Does not work with indirect traces
			//};
			cmd.barrier2(1, &globalRay);
			cmd.fill_buffer(*m_scratchBuffer, 0);
			//auto& srcbuf = r_renderManager.get_ddgi_manager().get_ray_count_device_addressB(i);
			//auto& srcBuf2 = r_renderManager.get_ddgi_manager().get_ray_count_device_addressC(i);
			//cmd.copy_buffer(*dstBuf, srcbuf);
			//cmd.copy_buffer(*dstBuf2, srcBuf2);
			//std::vector<uint32_t> dataA(uintPart);
			//std::vector<float> dataB(floatPart);
			//std::memcpy(dataA.data(), (*dstBuf2)->map_data(), uintPart * sizeof(uint32_t));
			//std::memcpy(dataB.data(), reinterpret_cast<float*>((*dstBuf2)->map_data()) + uintPart, floatPart * sizeof(uint32_t));
			////assert(dataA[25344] == 0 || dataA[25344] == 3244032);
			//assert(dataA[25344] == 0 || dataA[25344] == 3244032);
			//assert(dataA[25343] == 0 || dataA[25343] == 3244032);
			//auto b = *reinterpret_cast<VkTraceRaysIndirectCommandKHR*>((*dstBuf)->map_data());
			//assert(b.height == 0 || b.height == 3168);
			r_renderManager.get_profiler().end_profile(cmd);
			r_renderManager.get_profiler().begin_profile(cmd, "DDGI Ray Trace");
			auto rtPipelineBind = cmd.bind_raytracing_pipeline(rtPipeline);
			render_volume(rtPipelineBind, rtPipeline, constants, dynamicConstants.targetAddress);
			r_renderManager.get_profiler().end_profile(cmd);

		}
		else {
			r_renderManager.get_profiler().begin_profile(cmd, "DDGI Ray Trace");
			auto rtPipelineBind = cmd.bind_raytracing_pipeline(rtPipeline);
			render_volume(rtPipelineBind, rtPipeline, constants, (1ul << m_renderTargetWidthBits),
				(volume.raysPerProbe * volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((1ul << m_renderTargetWidthBits) - 1ul)) >> m_renderTargetWidthBits);
			r_renderManager.get_profiler().end_profile(cmd);
		}
		cmd.barrier2(1, &writeBarrier);

		filter_volume(cmd, volume, constants);

		relocate_probes(cmd, volume, constants, parameters.frames <= 1);

		copy_borders(cmd, volume, constants);

		if (volume.dynamicRayAllocationEnabled) {
			{
				r_renderManager.get_profiler().begin_profile(cmd, "DDGI Sum Variance");
				ScanPipelineConfig scanPipelineCfg{
					.workSizeX {m_scanGroupSize},
					.workSizeY {1},
					.workSizeZ {1},
					.subgroupSize {r_device.get_physical_device().get_subgroup_properties().subgroupSize},
					.numRows {m_scanNumRows}
				};
				auto scanPipeline = create_scan_pipeline(scanPipelineCfg);
				auto pipelineBind = cmd.bind_compute_pipeline(scanPipeline);
				sum_variance(pipelineBind, dynamicConstants, (volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((m_scanGroupSize * m_scanNumRows) - 1)) / (m_scanGroupSize * m_scanNumRows), 1, 1);
				r_renderManager.get_profiler().end_profile(cmd);
			}
			static constexpr VkMemoryBarrier2 global{
				.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
				.pNext {nullptr},
				.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
				.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
				.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
				.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
			};
			cmd.barrier2(1, &global);
			{
				r_renderManager.get_profiler().begin_profile(cmd, "DDGI Gather Variance");
				GatherPipelineConfig gatherPipelineCfg{
					.workSizeX {m_gatherGroupSize},
					.workSizeY {1},
					.workSizeZ {1},
					.numRows {m_gatherNumRows}
				};
				auto gatherPipeline = create_gather_pipeline(gatherPipelineCfg);
				auto pipelineBind = cmd.bind_compute_pipeline(gatherPipeline);
				cmd.fill_buffer(*m_scratchBuffer, 0);
				gather_variance(pipelineBind, dynamicConstants, (volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((m_gatherGroupSize * m_gatherNumRows) - 1)) / (m_gatherGroupSize * m_gatherNumRows), 1, 1);
				r_renderManager.get_profiler().end_profile(cmd);
			}
		}

	}
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const DDGIPushConstants& constants, uint32_t numRays, uint32_t numProbes)
{
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);

	//const auto& volume = ddgiManager.get(volumeId);

	bind.push_constants(constants);
	const std::array dispatch{
		numRays,
		numProbes,
		1u
	};
	assert(dispatch[0] * dispatch[1] * dispatch[2] <= r_device.get_physical_device().get_ray_tracing_pipeline_properties().maxRayDispatchInvocationCount);
	assert(dispatch[0] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[0]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatch[1] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[1]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatch[2] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[2]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);

	bind.trace_rays(pipeline, dispatch[0], dispatch[1], dispatch[2]);
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const DDGIPushConstants& constants, VkDeviceAddress address)
{
	bind.push_constants(constants);

	bind.trace_rays(pipeline, address);
}

void nyan::DDGIRenderer::filter_volume(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants)
{
	r_renderManager.get_profiler().begin_profile(cmd, "DDGI Filter");
	static constexpr VkMemoryBarrier2 global{
		.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
		.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
		.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
		.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
	};

	r_renderManager.get_profiler().begin_profile(cmd, "Irradiance");

	bind_and_dispatch_compute(cmd, create_filter_pipeline(PipelineConfig{
		.workSizeX {volume.irradianceProbeSize},
		.workSizeY {volume.irradianceProbeSize},
		.workSizeZ {1},
		.rayCount {volume.raysPerProbe},
		.filterIrradiance {true},
		.renderTargetImageFormat {volume.renderTargetImageFormat},
		.imageFormat {volume.irradianceImageFormat},
		.renderTargetImageWidthBits{m_renderTargetWidthBits},
		.dynamicRayAllocationEnabled {volume.dynamicRayAllocationEnabled}
		}), constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
	r_renderManager.get_profiler().end_profile(cmd);
	r_renderManager.get_profiler().begin_profile(cmd, "Depth");

	bind_and_dispatch_compute(cmd, create_filter_pipeline(PipelineConfig{
		.workSizeX {volume.depthProbeSize},
		.workSizeY {volume.depthProbeSize},
		.workSizeZ {1},
		.rayCount {volume.raysPerProbe},
		.filterIrradiance {false},
		.renderTargetImageFormat {volume.renderTargetImageFormat},
		.imageFormat {volume.depthImageFormat},
		.renderTargetImageWidthBits{m_renderTargetWidthBits},
		.dynamicRayAllocationEnabled {volume.dynamicRayAllocationEnabled}
		}), constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
	cmd.barrier2(1, &global);
	r_renderManager.get_profiler().end_profile(cmd);
	r_renderManager.get_profiler().end_profile(cmd);

}

void nyan::DDGIRenderer::copy_borders(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants)
{
	r_renderManager.get_profiler().begin_profile(cmd, "DDGI Copy Borders");
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {m_borderSizeX},
			.workSizeY {m_borderSizeY},
			.workSizeZ {1},
			.columns {true},
			.filterIrradiance {true},
			.imageFormat{volume.irradianceImageFormat},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.irradianceProbeSize},
			.imageBinding{volume.irradianceImageBinding}
		}), constants, static_cast<uint32_t>(std::ceil(volume.probeCountX / static_cast<float>(m_borderSizeX))),
			static_cast<uint32_t>(std::ceil(volume.irradianceTextureSizeY / static_cast<float>(m_borderSizeY))) , volume.probeCountZ);
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {m_borderSizeX},
			.workSizeY {m_borderSizeY},
			.workSizeZ {1},
			.columns {false},
			.filterIrradiance {true},
			.imageFormat{volume.irradianceImageFormat},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.irradianceProbeSize},
			.imageBinding{volume.irradianceImageBinding}
		}), constants, static_cast<uint32_t>(std::ceil(volume.irradianceTextureSizeX / static_cast<float>(m_borderSizeX))),
			static_cast<uint32_t>(std::ceil(volume.probeCountY / static_cast<float>(m_borderSizeY))), volume.probeCountZ);
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {m_borderSizeX},
			.workSizeY {m_borderSizeY},
			.workSizeZ {1},
			.columns {true},
			.filterIrradiance {false},
			.imageFormat{volume.depthImageFormat},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.depthProbeSize},
			.imageBinding{volume.depthImageBinding}
		}), constants, static_cast<uint32_t>(std::ceil(volume.probeCountX / static_cast<float>(m_borderSizeX))),
			static_cast<uint32_t>(std::ceil(volume.depthTextureSizeY / static_cast<float>(m_borderSizeY))), volume.probeCountZ);
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {m_borderSizeX},
			.workSizeY {m_borderSizeY},
			.workSizeZ {1},
			.columns {false},
			.filterIrradiance {false},
			.imageFormat{volume.depthImageFormat},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.depthProbeSize},
			.imageBinding{volume.depthImageBinding}
		}), constants, static_cast<uint32_t>(std::ceil(volume.depthTextureSizeX / static_cast<float>(m_borderSizeX))),
			static_cast<uint32_t>(std::ceil(volume.probeCountY / static_cast<float>(m_borderSizeY))), volume.probeCountZ);
	r_renderManager.get_profiler().end_profile(cmd);

}
void nyan::DDGIRenderer::dispatch_compute(vulkan::ComputePipelineBind& bind, const DDGIPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ)
{
	bind.push_constants(constants);
	assert(dispatchCountX <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatchCountY <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatchCountZ <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	bind.dispatch(dispatchCountX, dispatchCountY, dispatchCountZ);
}
void nyan::DDGIRenderer::bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, const DDGIPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ)
{
	auto bind = cmd.bind_compute_pipeline(pipelineId);
	bind.push_constants(constants);
	assert(dispatchCountX <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatchCountY <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatchCountZ <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	bind.dispatch(dispatchCountX, dispatchCountY, dispatchCountZ);
}

void nyan::DDGIRenderer::relocate_probes(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants, bool reset)
{
	if (!reset && !volume.relocationEnabled)
		return;

	r_renderManager.get_profiler().begin_profile(cmd, "DDGI Relocate");
	static constexpr uint32_t workSizeX = 32;
	bind_and_dispatch_compute(cmd, create_relocate_pipeline(RelocatePipelineConfig{
		.workSizeX {workSizeX},
		.workSizeY {1},
		.workSizeZ {1},
		.relocationEnabled {reset? VK_FALSE: VK_TRUE},
		.renderTargetImageWidthBits {m_renderTargetWidthBits},
		.renderTargetImageFormat {volume.renderTargetImageFormat},
		}), constants,
		(volume.probeCountX * volume.probeCountY * volume.probeCountZ + (workSizeX - 1))
		/ workSizeX, 1, 1);
	r_renderManager.get_profiler().end_profile(cmd);
	
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
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits },
			vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::dynamicRayAllocationEnabledShaderName, config.dynamicRayAllocationEnabled}
			)},
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
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::imageFormatShaderName, config.imageFormat},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountXShaderName, config.probeCountX},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountYShaderName, config.probeCountY},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountZShaderName, config.probeCountZ},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeSizeShaderName, config.probeSize},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::imageBindingShaderName, config.imageBinding}
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
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_gather_comp"
				,config.workSizeX
				,config.workSizeY
				,config.workSizeZ
				,vulkan::ShaderStorage::SpecializationConstant{GatherPipelineConfig::numRowsShaderName, config.numRows}
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
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_prefix_sum_comp"
				,config.workSizeX
				,config.workSizeY
				,config.workSizeZ
				,vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::numRowsShaderName, config.numRows}
				,vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::subgroupSizeShaderName, config.subgroupSize}
				,vulkan::ShaderStorage::SpecializationConstant{PrefixSumPipelineConfig::renderTargetImageWidthBitsShaderName, config.renderTargetImageWidthBits}
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
				,vulkan::ShaderStorage::SpecializationConstant{RTConfig::dynamicRayAllocationEnabledShaderName, config.dynamicRayAllocationEnabled}
				//,vulkan::ShaderStorage::SpecializationConstant{RTConfig::numRowsShaderName, config.numRows}
				)},
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
		.recursionDepth {1},
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
	float u = m_dist(*m_generator);
	float v = m_dist(*m_generator);
	float w = m_dist(*m_generator);
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
	vulkan::GraphicsPipelineConfig visualizerConfig {
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
		.vertexInputFormats {},
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


nyan::DDGIReSTIRRenderer::DDGIReSTIRRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	Renderer(device, registry, renderManager, pass),
	m_sampleGenerationPipeline(std::make_unique< vulkan::RTPipeline>(r_device, generate_sample_generation_config())),
	m_sampleValidationPipeline(std::make_unique< vulkan::RTPipeline>(r_device, generate_sample_validation_config()))
{
	auto& ddgiRestirManager = r_renderManager.get_ddgi_restir_manager();
	ddgiRestirManager.add_write(r_pass.get_id(), nyan::Renderpass::Write::Type::Compute);

	pass.add_renderfunction(std::bind(&DDGIReSTIRRenderer::render, this, std::placeholders::_1, std::placeholders::_2), false);
}

void nyan::DDGIReSTIRRenderer::begin_frame()
{
	uint32_t maxRays = 0;
	VkDeviceSize maxReservoirs = 0;
	auto& ddgiReSTIRManager = r_renderManager.get_ddgi_restir_manager();
	for (uint32_t i = 0; i < ddgiReSTIRManager.slot_count(); ++i) {
		const auto& volume = ddgiReSTIRManager.get(i);
		auto probeCount = volume.probeCountX * volume.probeCountY * volume.probeCountZ;
		auto rayCount = probeCount * volume.samplesPerProbe;
		VkDeviceSize reservoirCount = static_cast<VkDeviceSize>(probeCount) * volume.temporalReservoirCountX * volume.temporalReservoirCountY;
		if (rayCount > maxRays)
			maxRays = rayCount;
		if (reservoirCount > maxReservoirs)
			maxReservoirs = reservoirCount;
		assert(volume.temporalReservoirCountX == volume.irradianceProbeSize);
		assert(volume.temporalReservoirCountY == volume.irradianceProbeSize);
	}

	auto& renderGraph = r_renderManager.get_render_graph();
	auto width = static_cast<float>(1 << 11);
	auto height = std::ceil(static_cast<float>(maxRays) / static_cast<float>(1 << 11));
	if (!m_renderTarget) {
		m_renderTarget = renderGraph.add_ressource("DDGI_ReSTIR_RenderTarget", nyan::ImageAttachment
			{
				.format{VK_FORMAT_R32G32B32A32_SFLOAT},//VK_FORMAT_R16G16B16A16_SFLOAT},
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
		image.format = VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	auto reservoirSize = sizeof(nyan::shaders::DDGIReSTIRTemporalReservoir) * maxReservoirs;
	if (!m_temporalReservoirs) {
		m_temporalReservoirs = std::make_unique<vulkan::BufferHandle>(r_device.create_buffer(vulkan::BufferInfo{ .size{reservoirSize},
			.usage {VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT },.offset{}, .memoryUsage{VMA_MEMORY_USAGE_AUTO} }, {}));
		m_dirtyReservoirs = true;
	}
	else {
		if (m_dirtyReservoirs = ((*m_temporalReservoirs)->get_size() != reservoirSize); m_dirtyReservoirs)
			(*m_temporalReservoirs)->resize(reservoirSize);
	}
}

void nyan::DDGIReSTIRRenderer::dump_to_disk()
{
	m_dumpToDisk = true;
}

void nyan::DDGIReSTIRRenderer::clear_buffers()
{
	m_clear = true;
}

static constexpr uint32_t jenkins_hash(uint32_t x) {
	x += (x << 10u);
	x ^= (x >> 6u);
	x += (x << 3u);
	x ^= (x >> 11u);
	x += (x << 15u);
	return x;
}
static constexpr uint32_t hash_mut(uint32_t& h) {
	auto res = h;
	h = jenkins_hash(h);
	return res;
}

static constexpr float uint_to_u01_float(uint32_t h) {
	constexpr uint32_t mantissaMask = 0x007FFFFFu;
	constexpr uint32_t one = std::bit_cast<uint32_t>(1.f);

	h &= mantissaMask;
	h |= one;

	float r2 = std::bit_cast<float>(h);
	return r2 - 1.0f;
}

void nyan::DDGIReSTIRRenderer::render(vulkan::CommandBuffer& cmd, nyan::Renderpass&)
{
	if (!m_enabled)
		return;
	const auto& ddgiReSTIRManager = r_renderManager.get_ddgi_restir_manager();
	if (!ddgiReSTIRManager.slot_count())
		return;
	if (m_dirtyReservoirs || m_clear) {
		cmd.fill_buffer(*m_temporalReservoirs, 0);
		m_dirtyReservoirs = false;
		m_clear = false;
	}
	auto& res = r_pass.get_graph().get_resource(m_renderTarget);
	if (m_screenshot) {
		auto& img = *res.handle;
		m_screenshot = false;
		auto* data = (*m_screenshotBuffer)->map_data();
		stbi_write_hdr("DDGI_ReSTIR_SAMPLES.hdr", m_screenshotWidth, m_screenshotHeight, 4, reinterpret_cast<const float*>(data));
	}

	auto rngSeed = (*m_generator)();
	uint32_t tmpSeed = rngSeed;
	float u = uint_to_u01_float(hash_mut(tmpSeed));
	float v = uint_to_u01_float(hash_mut(tmpSeed));
	float w = uint_to_u01_float(hash_mut(tmpSeed));

	DDGIReSTIRPushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()},
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()},
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()},
		.ddgiReSTIRBinding {ddgiReSTIRManager.get_binding()},
		.ddgiReSTIRCount {static_cast<uint32_t>(ddgiReSTIRManager.slot_count())},
		.ddgiReSTIRIndex {0},
		.renderTarget {r_pass.get_write_bind(m_renderTarget, nyan::Renderpass::Write::Type::Compute)},
		.rngSeed {rngSeed},
		.reservoirs {(*m_temporalReservoirs)->get_address()},
		.randomRotation {sqrt(1 - u) * sin(Math::pi_2 * v), sqrt(1 - u) * cos(Math::pi_2 * v),
							sqrt(u) * sin(Math::pi_2 * w), sqrt(u) * cos(Math::pi_2 * w)},
	};

	const auto& deviceVolume = ddgiReSTIRManager.get(0);
	generate_samples(cmd, deviceVolume, constants);
	if(deviceVolume.validationEnabled)
		validate_samples(cmd, deviceVolume, constants);

	cmd.barrier2(VkMemoryBarrier2{
		.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
		.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
		.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
		.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
	});



	

	if (res.handle && m_dumpToDisk) {
		cmd.barrier2(VkMemoryBarrier2{
			.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
			.pNext {nullptr},
			.srcStageMask {VK_PIPELINE_STAGE_2_RAY_TRACING_SHADER_BIT_KHR},
			.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
			.dstStageMask {VK_PIPELINE_STAGE_2_TRANSFER_BIT},
			.dstAccessMask {VK_ACCESS_2_MEMORY_READ_BIT},
		});
		m_dumpToDisk = false;
		m_screenshot = true;
		auto& img = *res.handle;
		std::vector copy{ VkBufferImageCopy{
			.bufferOffset {0},
			.bufferRowLength {0},
			.bufferImageHeight {0},
			.imageSubresource {
				.aspectMask {VK_IMAGE_ASPECT_COLOR_BIT},
				.mipLevel {0},
				.baseArrayLayer {0},
				.layerCount {1}
			},
			.imageOffset {0, 0, 0},
			.imageExtent {img.get_info().width, img.get_info().height, img.get_info().depth},
		} };
		m_screenshotWidth = img.get_info().width;
		m_screenshotHeight = img.get_info().height;
		auto bufferSize = vulkan::format_block_size(img.get_format()) * img.get_info().width * img.get_info().height * img.get_info().depth;
		if (!m_screenshotBuffer || (*m_screenshotBuffer)->get_size() < bufferSize) {
			
			m_screenshotBuffer = std::make_unique<vulkan::BufferHandle>(r_device.create_buffer(vulkan::BufferInfo{
				.size {bufferSize},
				.usage {VK_BUFFER_USAGE_TRANSFER_DST_BIT },
				.offset {0},
				.memoryUsage {VMA_MEMORY_USAGE_CPU_ONLY},
				}, {}));
		}
		cmd.copy_image_to_buffer(img, *m_screenshotBuffer, copy, VK_IMAGE_LAYOUT_GENERAL);
	}


	resample(cmd, deviceVolume, constants);

	//cmd.barrier2(VkMemoryBarrier2{
	//	.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
	//	.pNext {nullptr},
	//	.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
	//	.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
	//	.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
	//	.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
	//});

	//shade(cmd, deviceVolume, constants);

	cmd.barrier2(VkMemoryBarrier2 {
		.sType {VK_STRUCTURE_TYPE_MEMORY_BARRIER_2},
		.pNext {nullptr},
		.srcStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
		.srcAccessMask {VK_ACCESS_2_SHADER_WRITE_BIT},
		.dstStageMask {VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT},
		.dstAccessMask {VK_ACCESS_2_SHADER_READ_BIT},
	});

	copy_borders(cmd, deviceVolume);
}

vulkan::PipelineId nyan::DDGIReSTIRRenderer::create_filter_pipeline(const PipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_filterPipelines.find(config); it != m_filterPipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_restir_update_comp"
				,config.workSizeX
				,config.workSizeY
				,config.workSizeZ
				,vulkan::ShaderStorage::SpecializationConstant{PipelineConfig::sampleCountShaderName, config.sampleCount}
				)},
					.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_filterPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}

vulkan::PipelineId nyan::DDGIReSTIRRenderer::create_shade_pipeline(const ShadePipelineConfig& config)
{
	vulkan::PipelineId pipelineId;
	if (auto it = m_shadePipelines.find(config); it != m_shadePipelines.end()) {
		pipelineId = it->second;
	}
	else {
		vulkan::ComputePipelineConfig pipelineConfig{
				.shaderInstance {r_renderManager.get_shader_manager().get_shader_instance_id_workgroup_size("ddgi_restir_update_comp"
				,config.workSizeX
				,config.workSizeY
				,config.workSizeZ
					//,vulkan::ShaderStorage::SpecializationConstant{GatherPipelineConfig::numRowsShaderName, config.numRows}
					)},
					.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_shadePipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}

vulkan::PipelineId nyan::DDGIReSTIRRenderer::create_border_pipeline(const BorderPipelineConfig& config)
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
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::imageFormatShaderName, config.imageFormat},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountXShaderName, config.probeCountX},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountYShaderName, config.probeCountY},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeCountZShaderName, config.probeCountZ},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::probeSizeShaderName, config.probeSize},
				vulkan::ShaderStorage::SpecializationConstant{BorderPipelineConfig::imageBindingShaderName, config.imageBinding}
				)},
				.pipelineLayout {r_device.get_bindless_pipeline_layout()}
		};
		pipelineId = r_device.get_pipeline_storage().add_pipeline(pipelineConfig);
		m_borderPipelines.emplace(config, pipelineId);
	}
	return pipelineId;
}

vulkan::RaytracingPipelineConfig nyan::DDGIReSTIRRenderer::generate_sample_generation_config()
{
	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_sample_generation_rgen"
				//,vulkan::ShaderStorage::SpecializationConstant{RTConfig::renderTargetImageFormatShaderName, config.renderTargetImageFormat}
				)},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_raytrace_rchit")},
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_alpha_test_rahit")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_raytrace_rmiss")},
			},
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_shadows_rmiss")},
			},
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}

vulkan::RaytracingPipelineConfig nyan::DDGIReSTIRRenderer::generate_sample_validation_config()
{
	return vulkan::RaytracingPipelineConfig{
		.rgenGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_sample_validation_rgen"
				//,vulkan::ShaderStorage::SpecializationConstant{RTConfig::renderTargetImageFormatShaderName, config.renderTargetImageFormat}
				)},
			},
		},
		.hitGroups {
			vulkan::Group
			{
				.closestHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_raytrace_rchit")},
				.anyHitShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_alpha_test_rahit")},
			},
		},
		.missGroups {
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("ddgi_restir_raytrace_rmiss")},
			},
			vulkan::Group
			{
				.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_shadows_rmiss")},
			},
		},
		.recursionDepth {1},
		.pipelineLayout = r_device.get_bindless_pipeline_layout()
	};
}

void nyan::DDGIReSTIRRenderer::generate_samples(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants)
{

	r_renderManager.get_profiler().begin_profile(cmd, "Generate Samples");
	static constexpr auto renderTargetWidth = 11ul;
	const std::array dispatch{
		1ul << renderTargetWidth,
		(volume.samplesPerProbe * volume.probeCountX * volume.probeCountY * volume.probeCountZ + ((1ul << renderTargetWidth) - 1ul)) >> renderTargetWidth,
		1ul
	};
	assert(dispatch[0] * dispatch[1] * dispatch[2] <= r_device.get_physical_device().get_ray_tracing_pipeline_properties().maxRayDispatchInvocationCount);
	assert(dispatch[0] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[0]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatch[1] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[1]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatch[2] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[2]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	assert(dispatch[0] > 0 && dispatch[1] > 0 &&dispatch[2] > 0);
	auto bind = cmd.bind_raytracing_pipeline(*m_sampleGenerationPipeline);
	bind.push_constants(constants);
	bind.trace_rays(*m_sampleGenerationPipeline, dispatch[0], dispatch[1], dispatch[2]);
	r_renderManager.get_profiler().end_profile(cmd);
}

void nyan::DDGIReSTIRRenderer::validate_samples(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants)
{
	r_renderManager.get_profiler().begin_profile(cmd, "Validate Samples");
	const std::array dispatch{
		volume.probeCountX * volume.irradianceProbeSize,
		volume.probeCountY * volume.irradianceProbeSize,
		volume.probeCountZ
	};
	assert(dispatch[0] * dispatch[1] * dispatch[2] <= r_device.get_physical_device().get_ray_tracing_pipeline_properties().maxRayDispatchInvocationCount);
	assert(dispatch[0] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[0]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatch[1] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[1]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatch[2] <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupSize[2]
		* r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	assert(dispatch[0] > 0 && dispatch[1] > 0 && dispatch[2] > 0);
	auto bind = cmd.bind_raytracing_pipeline(*m_sampleValidationPipeline);
	bind.push_constants(constants);
	bind.trace_rays(*m_sampleValidationPipeline, dispatch[0], dispatch[1], dispatch[2]);
	r_renderManager.get_profiler().end_profile(cmd);
}

void nyan::DDGIReSTIRRenderer::resample(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants)
{

	r_renderManager.get_profiler().begin_profile(cmd, "Resampling");
	bind_and_dispatch_compute(cmd, create_filter_pipeline(PipelineConfig{
		.workSizeX {volume.irradianceProbeSize},
		.workSizeY {volume.irradianceProbeSize},
		.workSizeZ {1},
		.sampleCount {volume.samplesPerProbe}
		}), constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
	r_renderManager.get_profiler().end_profile(cmd);
}

void nyan::DDGIReSTIRRenderer::copy_borders(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume)
{
	static constexpr uint32_t borderWorkSizeX = 8;
	static constexpr uint32_t borderWorkSizeY = 8;
	r_renderManager.get_profiler().begin_profile(cmd, "Copy Borders");
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {borderWorkSizeX},
			.workSizeY {borderWorkSizeY},
			.workSizeZ {1},
			.columns {true},
			.filterIrradiance {true},
			.imageFormat{nyan::shaders::R16G16B16A16F},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.irradianceProbeSize},
			.imageBinding{volume.irradianceImageBinding}
		}), static_cast<uint32_t>(std::ceil(volume.probeCountX / static_cast<float>(borderWorkSizeX))),
		static_cast<uint32_t>(std::ceil(volume.irradianceTextureSizeY / static_cast<float>(borderWorkSizeY))), volume.probeCountZ);
	bind_and_dispatch_compute(cmd, create_border_pipeline(
		BorderPipelineConfig{
			.workSizeX {borderWorkSizeX},
			.workSizeY {borderWorkSizeY},
			.workSizeZ {1},
			.columns {false},
			.filterIrradiance {true},
			.imageFormat{nyan::shaders::R16G16B16A16F},
			.probeCountX{volume.probeCountX},
			.probeCountY{volume.probeCountY},
			.probeCountZ{volume.probeCountZ},
			.probeSize{2 + volume.irradianceProbeSize},
			.imageBinding{volume.irradianceImageBinding}
		}), static_cast<uint32_t>(std::ceil(volume.irradianceTextureSizeX / static_cast<float>(borderWorkSizeX))),
		static_cast<uint32_t>(std::ceil(volume.probeCountY / static_cast<float>(borderWorkSizeY))), volume.probeCountZ);
	r_renderManager.get_profiler().end_profile(cmd);
}

void nyan::DDGIReSTIRRenderer::shade(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants)
{
	bind_and_dispatch_compute(cmd, create_shade_pipeline(ShadePipelineConfig{
		.workSizeX {volume.irradianceProbeSize},
		.workSizeY {volume.irradianceProbeSize},
		.workSizeZ {1},
		}), constants, volume.probeCountX, volume.probeCountY, volume.probeCountZ);
}

void nyan::DDGIReSTIRRenderer::dispatch_compute(vulkan::ComputePipelineBind& bind, const DDGIReSTIRPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ)
{
	bind.push_constants(constants);
	assert(dispatchCountX <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatchCountY <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatchCountZ <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	assert(dispatchCountX > 0 && dispatchCountY > 0 && dispatchCountZ > 0);
	bind.dispatch(dispatchCountX, dispatchCountY, dispatchCountZ);
}

void nyan::DDGIReSTIRRenderer::bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, const DDGIReSTIRPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ)
{
	auto bind = cmd.bind_compute_pipeline(pipelineId);
	bind.push_constants(constants);
	assert(dispatchCountX <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatchCountY <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatchCountZ <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	assert(dispatchCountX > 0 && dispatchCountY > 0 && dispatchCountZ > 0);
	bind.dispatch(dispatchCountX, dispatchCountY, dispatchCountZ);
}

void nyan::DDGIReSTIRRenderer::bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ)
{
	auto bind = cmd.bind_compute_pipeline(pipelineId);
	assert(dispatchCountX <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[0]);
	assert(dispatchCountY <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[1]);
	assert(dispatchCountZ <= r_device.get_physical_device().get_properties().limits.maxComputeWorkGroupCount[2]);
	assert(dispatchCountX > 0 && dispatchCountY > 0 && dispatchCountZ > 0);
	bind.dispatch(dispatchCountX, dispatchCountY, dispatchCountZ);
}
