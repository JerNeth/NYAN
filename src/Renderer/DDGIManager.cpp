#include "Renderer/DDGIManager.h"
#include "Buffer.h"
#include "LogicalDevice.h"

#include "entt/entt.hpp"

nyan::DDGIManager::DDGIManager(vulkan::LogicalDevice& device, entt::registry& registry) :
	DataManager(device),
	r_registry(registry)
{
}

uint32_t nyan::DDGIManager::add_ddgi_volume(const DDGIVolumeParameters& parameters)
{

	nyan::shaders::DDGIVolume volume{
			.spacingX {parameters.spacing[0]},
			.spacingY {parameters.spacing[1]},
			.spacingZ {parameters.spacing[2]},
			.inverseSpacingX {},
			.inverseSpacingY {},
			.inverseSpacingZ {},
			.gridOriginX {parameters.origin[0]},
			.gridOriginY {parameters.origin[1]},
			.gridOriginZ {parameters.origin[2]},
			.probeCountX {parameters.probeCount[0]},
			.probeCountY {parameters.probeCount[1]},
			.probeCountZ {parameters.probeCount[2]},
			.raysPerProbe {parameters.raysPerProbe},
			.irradianceProbeSize {parameters.irradianceProbeSize},
			.depthProbeSize {parameters.depthProbeSize},
			.irradianceTextureSizeX {},
			.irradianceTextureSizeY {},
			.inverseIrradianceTextureSizeX {},
			.inverseIrradianceTextureSizeY {},
			.irradianceTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.irradianceImageBinding {},
			.depthTextureSizeX {},
			.depthTextureSizeY {},
			.inverseDepthTextureSizeX {},
			.inverseDepthTextureSizeY {},
			.depthTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.depthImageBinding {},
			.shadowBias {parameters.depthBias}
	};
	update_spacing(volume);
	update_depth_texture(volume);
	update_irradiance_texture(volume);
	return add(volume);
}

void nyan::DDGIManager::set_spacing(uint32_t id, const Math::vec3& spacing)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.spacingX = spacing[0];
	volume.spacingY = spacing[1];
	volume.spacingZ = spacing[2];
	update_spacing(volume);
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
	update_depth_texture(volume);
	update_irradiance_texture(volume);

}

void nyan::DDGIManager::set_irradiance_probe_size(uint32_t id, uint32_t probeSize)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.irradianceProbeSize = probeSize;
	update_irradiance_texture(volume);
}

void nyan::DDGIManager::set_depth_probe_size(uint32_t id, uint32_t probeSize)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.depthProbeSize = probeSize;
	update_depth_texture(volume);
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

void nyan::DDGIManager::update()
{

	auto volumeView = r_registry.view<const DDGIVolumeParameters>();
	for (const auto& [entity, parameters] : volumeView.each()) {
		auto& deviceVolume = DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume);
		deviceVolume = nyan::shaders::DDGIVolume{
			.spacingX {parameters.spacing[0]},
			.spacingY {parameters.spacing[1]},
			.spacingZ {parameters.spacing[2]},
			.gridOriginX {parameters.origin[0]},
			.gridOriginY {parameters.origin[1]},
			.gridOriginZ {parameters.origin[2]},
			.probeCountX {parameters.probeCount[0]},
			.probeCountY {parameters.probeCount[1]},
			.probeCountZ {parameters.probeCount[2]},
			.raysPerProbe {parameters.raysPerProbe},
			.irradianceProbeSize {parameters.irradianceProbeSize},
			.depthProbeSize {parameters.depthProbeSize},
			.irradianceTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.depthTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.shadowBias {parameters.depthBias}
		};
		update_spacing(deviceVolume);
		update_depth_texture(deviceVolume);
		update_irradiance_texture(deviceVolume);
		//TODO	update texture References must be done after rendergraph "update"
		//		update/setup texture read/writes must be done before rendergraph "update"
	}
}

void nyan::DDGIManager::update_spacing(nyan::shaders::DDGIVolume& volume)
{
	volume.spacingX = Math::max(volume.spacingX, 1e-6f);
	volume.spacingY = Math::max(volume.spacingY, 1e-6f);
	volume.spacingZ = Math::max(volume.spacingZ, 1e-6f);
	volume.inverseSpacingX = 1.0f / volume.spacingX;
	volume.inverseSpacingY = 1.0f / volume.spacingY;
	volume.inverseSpacingZ = 1.0f / volume.spacingZ;
}


void nyan::DDGIManager::update_depth_texture(nyan::shaders::DDGIVolume& volume)
{
	//Add 1 pixel sample border
	volume.depthTextureSizeX = volume.probeCountX * (2 + volume.depthProbeSize);
	volume.depthTextureSizeY = volume.probeCountY * volume.probeCountY * (2 + volume.depthProbeSize);
	volume.inverseDepthTextureSizeX = 1.0f / volume.depthTextureSizeX;
	volume.inverseDepthTextureSizeY = 1.0f / volume.depthTextureSizeY;
}

void nyan::DDGIManager::update_irradiance_texture(nyan::shaders::DDGIVolume& volume)
{
	//Add 1 pixel sample border
	volume.irradianceTextureSizeX = volume.probeCountX * (2 + volume.irradianceProbeSize);
	volume.irradianceTextureSizeY = volume.probeCountY * volume.probeCountY * (2 + volume.irradianceProbeSize);
	volume.inverseIrradianceTextureSizeX = 1.0f / volume.irradianceTextureSizeX;
	volume.inverseIrradianceTextureSizeY = 1.0f / volume.irradianceTextureSizeY;
}