#include "Renderer/DDGIRenderer.h"
#include "Renderer/RenderManager.h"
#include "VulkanWrapper/Buffer.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/CommandBuffer.h"

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
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.spacingX = spacing[0];
	volume.spacingY = spacing[1];
	volume.spacingZ = spacing[2];
	volume.inverseSpacingX = 1.0f / spacing[0];
	volume.inverseSpacingY = 1.0f / spacing[1];
	volume.inverseSpacingZ = 1.0f / spacing[2];
}

void nyan::DDGIManager::set_origin(uint32_t id, const Math::vec3& origin)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.gridOriginX = origin[0];
	volume.gridOriginY = origin[1];
	volume.gridOriginZ = origin[2];
}

void nyan::DDGIManager::set_probe_count(uint32_t id, const Math::uvec3& probeCount)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
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

void nyan::DDGIManager::set_irradiance_probe_size(uint32_t id, uint32_t probeSize)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.irradianceProbeSize = probeSize;
	volume.irradianceTextureSizeX = volume.probeCountX * volume.irradianceProbeSize;
	volume.irradianceTextureSizeY = volume.probeCountY * volume.probeCountY * volume.irradianceProbeSize;
	volume.inverseIrradianceTextureSizeX = 1.0f / volume.irradianceTextureSizeX;
	volume.inverseIrradianceTextureSizeY = 1.0f / volume.irradianceTextureSizeY;
}

void nyan::DDGIManager::set_depth_probe_size(uint32_t id, uint32_t probeSize)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.depthProbeSize = probeSize;
	volume.depthTextureSizeX = volume.probeCountX * volume.depthProbeSize;
	volume.depthTextureSizeY = volume.probeCountY * volume.probeCountY * volume.depthProbeSize;
	volume.inverseDepthTextureSizeX = 1.0f / volume.depthTextureSizeX;
	volume.inverseDepthTextureSizeY = 1.0f / volume.depthTextureSizeY;
}

void nyan::DDGIManager::set_rays_per_probe(uint32_t id, uint32_t rayCount)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.raysPerProbe = rayCount;
}

void nyan::DDGIManager::set_depth_bias(uint32_t id, float depthBias)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
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
	m_renderDDGIPipeline(create_pipeline())
{

	pass.add_renderfunction([this](vulkan::CommandBufferHandle& cmd, nyan::Renderpass&)
		{
			auto pipelineBind = cmd->bind_compute_pipeline(m_renderDDGIPipeline);
			render_volume(pipelineBind, 0);
		}, false);
}

void nyan::DDGIRenderer::render_volume(vulkan::ComputePipelineBind& bind, uint32_t volumeId)
{
	const auto& ddgi_manager = r_renderManager.get_ddgi_manager();
	//auto writeBind = r_pass.get_write_bind("swap", nyan::Renderpass::Write::Type::Compute);
	//assert(writeBind != InvalidResourceId);
	PushConstants constants{
		.accBinding {*r_renderManager.get_instance_manager().get_tlas_bind()}, //0
		.sceneBinding {r_renderManager.get_scene_manager().get_binding()}, //3
		.meshBinding {r_renderManager.get_mesh_manager().get_binding()}, //1
		.ddgiBinding {ddgi_manager.get_binding()},
		.ddgiCount {static_cast<uint32_t>(ddgi_manager.slot_count())},
		.ddgiIndex {volumeId},
		//.col {},
		//.col2 {}
	};
	const auto& volume = ddgi_manager.get(volumeId);
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
