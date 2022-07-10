#include "Renderer/DDGIRenderer.h"
#include "entt/entt.hpp"


nyan::DDGIManager::DDGIManager(vulkan::LogicalDevice& device) :
	DataManager(device)
{
}

uint32_t nyan::DDGIManager::add_ddgi_volume(const nyan::shaders::DDGIVolume& volume)
{
	//nyan::shaders::DDGIVolume{
	//		.spacingX {},
	//		.spacingY {},
	//		.spacingZ {},
	//		.inverseSpacingX {},
	//		.inverseSpacingY {},
	//		.inverseSpacingZ {},
	//		.gridOriginX {},
	//		.gridOriginY {},
	//		.gridOriginZ {},
	//		.probeCountX {},
	//		.probeCountY {},
	//		.probeCountZ {},
	//		.raysPerProbe {},
	//		.irradianceProbeSize {},
	//		.depthProbeSize {},
	//		.irradianceTextureSizeX {},
	//		.irradianceTextureSizeY {},
	//		.inverseIrradianceTextureSizeX {},
	//		.inverseIrradianceTextureSizeY {},
	//		.irradianceTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
	//		.irradianceImageBinding {},
	//		.depthTextureSizeX {},
	//		.depthTextureSizeY {},
	//		.inverseDepthTextureSizeX {},
	//		.inverseDepthTextureSizeY {},
	//		.depthTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
	//		.depthImageBinding {},
	//		.shadowBias {}
	//};
	return add(volume);
}

void nyan::DDGIManager::set_spacing(uint32_t id, const Math::vec3& spacing)
{
	auto& volume = DataManager::get(id);
	volume.spacingX = spacing[0];
	volume.spacingY = spacing[1];
	volume.spacingZ = spacing[2];
	volume.inverseSpacingX = 1.0f / spacing[0];
	volume.inverseSpacingY = 1.0f / spacing[1];
	volume.inverseSpacingZ = 1.0f / spacing[2];
}

void nyan::DDGIManager::set_origin(uint32_t id, const Math::vec3& origin)
{
	auto& volume = DataManager::get(id);
	volume.gridOriginX = origin[0];
	volume.gridOriginY = origin[1];
	volume.gridOriginZ = origin[2];
}

void nyan::DDGIManager::set_probe_count(uint32_t id, const Math::uvec3& probeCount)
{
	auto& volume = DataManager::get(id);
	volume.probeCountX = probeCount[0];
	volume.probeCountY = probeCount[1];
	volume.probeCountZ = probeCount[2];
	volume.depthTextureSizeX = volume.probeCountX * volume.depthProbeSize;
	volume.depthTextureSizeY = volume.probeCountY * volume.probeCountY * volume.depthProbeSize;
	volume.irradianceTextureSizeX = volume.probeCountX * volume.irradianceProbeSize;
	volume.irradianceTextureSizeY = volume.probeCountY * volume.probeCountY * volume.irradianceProbeSize;
	volume.inverseDepthTextureSizeX = 1.0f / volume.depthTextureSizeX;
	volume.inverseDepthTextureSizeY = 1.0f / volume.depthTextureSizeY;
	volume.inverseIrradianceTextureSizeX = 1.0f / volume.irradianceTextureSizeX;
	volume.inverseIrradianceTextureSizeY = 1.0f / volume.irradianceTextureSizeY;
}

void nyan::DDGIManager::set_irradiance_probe_size(uint32_t id, float probeSize)
{
	auto& volume = DataManager::get(id);
	volume.irradianceProbeSize = probeSize;
	volume.irradianceTextureSizeX = volume.probeCountX * volume.irradianceProbeSize;
	volume.irradianceTextureSizeY = volume.probeCountY * volume.probeCountY * volume.irradianceProbeSize;
	volume.inverseIrradianceTextureSizeX = 1.0f / volume.irradianceTextureSizeX;
	volume.inverseIrradianceTextureSizeY = 1.0f / volume.irradianceTextureSizeY;
}

void nyan::DDGIManager::set_depth_probe_size(uint32_t id, float probeSize)
{
	auto& volume = DataManager::get(id);
	volume.depthProbeSize = probeSize;
	volume.depthTextureSizeX = volume.probeCountX * volume.depthProbeSize;
	volume.depthTextureSizeY = volume.probeCountY * volume.probeCountY * volume.depthProbeSize;
	volume.inverseDepthTextureSizeX = 1.0f / volume.depthTextureSizeX;
	volume.inverseDepthTextureSizeY = 1.0f / volume.depthTextureSizeY;
}

void nyan::DDGIManager::set_rays_per_probe(uint32_t id, uint32_t rayCount)
{
	auto& volume = DataManager::get(id);
	volume.raysPerProbe = rayCount;
}

void nyan::DDGIManager::set_depth_bias(uint32_t id, float depthBias)
{
	auto& volume = DataManager::get(id);
	volume.shadowBias = depthBias;
}

const nyan::shaders::DDGIVolume& nyan::DDGIManager::get(uint32_t id) const
{
	return DataManager<nyan::shaders::DDGIVolume>::get(id);
}

nyan::DDGIRenderer::DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass) :
	r_device(device),
	r_registry(registry),
	r_renderManager(renderManager),
	r_pass(pass),
	m_rtPipeline(create_pipeline(generate_config())),
	m_sbt(create_sbt(generate_config()))
{
}

void nyan::DDGIRenderer::render_volume(vulkan::RaytracingPipelineBind& bind, uint32_t volumeId)
{
	const auto& ddgi_manager = r_renderManager.get_ddgi_manager();
	auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	assert(writeBind != InvalidResourceId);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.ddgiBinding {ddgi_manager.get_binding()},
		.ddgiCount {ddgi_manager.slot_count()},
		.ddgiIndex {volumeId},
		//.col {},
		//.col2 {}
	};
	const auto& volume = ddgi_manager.get(volumeId);
	bind.push_constants(constants);
	bind.trace_rays(&m_rgenRegion, &m_missRegion, &m_hitRegion, &m_callableRegion, 1920, 1080, 1);
}

vulkan::RaytracingPipelineConfig nyan::DDGIRenderer::generate_config()
{
	return vulkan::RaytracingPipelineConfig{
	.rgenGroups {
		vulkan::Group
		{
			.generalShader {r_renderManager.get_shader_manager().get_shader_instance_id("raytrace_rgen")},
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

vulkan::PipelineId nyan::DDGIRenderer::create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig)
{
	return r_device.get_pipeline_storage().add_pipeline(rayConfig);
}

vulkan::BufferHandle nyan::DDGIRenderer::create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig)
{

	auto* pipeline = r_device.get_pipeline_storage().get_pipeline(m_rtPipeline);

	const auto& rtProperties = r_device.get_physical_device().get_ray_tracing_pipeline_properties();
	auto handleSize{ rtProperties.shaderGroupHandleSize };
	auto groupCount{ rayConfig.rgenGroups.size() + rayConfig.hitGroups.size() + rayConfig.missGroups.size() + rayConfig.callableGroups.size() };
	std::vector<std::byte> handleData(handleSize * groupCount);
	auto result = vkGetRayTracingShaderGroupHandlesKHR(r_device, *pipeline, 0, static_cast<uint32_t>(groupCount), handleData.size(), handleData.data());
	assert(result == VK_SUCCESS);
	if (result != VK_SUCCESS)
		throw std::runtime_error("Couldn't create SBT");
	auto handleStride = Utility::align_up(handleSize, rtProperties.shaderGroupHandleAlignment);
	uint32_t rgenCount{ static_cast<uint32_t>(rayConfig.rgenGroups.size()) };
	uint32_t hitCount{ static_cast<uint32_t>(rayConfig.hitGroups.size()) };
	uint32_t missCount{ static_cast<uint32_t>(rayConfig.missGroups.size()) };
	uint32_t callableCount{ static_cast<uint32_t>(rayConfig.callableGroups.size()) };

	assert(rgenCount == 1); //only one rgen per sbt, possibly per pipeline but idk
	m_rgenRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = Utility::align_up(handleStride, rtProperties.shaderGroupBaseAlignment),
		.size = Utility::align_up(rgenCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_hitRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(hitCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_missRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(missCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};
	m_callableRegion = VkStridedDeviceAddressRegionKHR
	{
		.stride = handleStride,
		.size = Utility::align_up(callableCount * handleStride, rtProperties.shaderGroupBaseAlignment),
	};

	assert(m_rgenRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_hitRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_missRegion.stride <= rtProperties.maxShaderGroupStride);
	assert(m_callableRegion.stride <= rtProperties.maxShaderGroupStride);
	auto bufferSize = m_rgenRegion.size + m_hitRegion.size + m_missRegion.size + m_callableRegion.size;

	std::vector<std::byte> stridedHandles(bufferSize);
	std::vector<vulkan::InputData> inputData{
		vulkan::InputData{
			.ptr {stridedHandles.data()},
			.size {bufferSize},
		}
	};
	size_t offset{ 0 };
	size_t handleCount{ 0 };
	for (size_t i{ 0 }; i < rgenCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_rgenRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_rgenRegion.size;
	for (size_t i{ 0 }; i < hitCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_hitRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_hitRegion.size;
	for (size_t i{ 0 }; i < missCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_missRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_missRegion.size;
	for (size_t i{ 0 }; i < callableCount; ++i, ++handleCount) {
		std::memcpy(stridedHandles.data() + offset + i * m_callableRegion.stride, handleData.data() + handleCount * handleSize, handleSize);
	}
	offset += m_callableRegion.size;


	auto sbt = r_device.create_buffer(vulkan::BufferInfo{
		.size = bufferSize,
		.usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY
		}, inputData);

	auto buffAddr = sbt->get_address();

	if (rgenCount)
		m_rgenRegion.deviceAddress = buffAddr;
	if (hitCount)
		m_hitRegion.deviceAddress = buffAddr + m_rgenRegion.size;
	if (missCount)
		m_missRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size;
	if (callableCount)
		m_callableRegion.deviceAddress = buffAddr + m_rgenRegion.size + m_hitRegion.size + m_missRegion.size;

	return sbt;
}
