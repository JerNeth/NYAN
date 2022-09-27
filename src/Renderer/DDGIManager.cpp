#include "Renderer/DDGIManager.h"
#include "Renderer/RenderGraph.h"
#include "Buffer.h"
#include "LogicalDevice.h"

#include "entt/entt.hpp"

nyan::DDGIManager::DDGIManager(vulkan::LogicalDevice& device, nyan::Rendergraph& rendergraph, entt::registry& registry) :
	DataManager(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, 32),
	r_rendergraph(rendergraph),
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
			.irradianceTextureBinding {},
			.irradianceTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.irradianceImageBinding {},
			.depthTextureSizeX {},
			.depthTextureSizeY {},
			.inverseDepthTextureSizeX {},
			.inverseDepthTextureSizeY {},
			.depthTextureBinding {},
			.depthTextureSampler {static_cast<uint32_t>(vulkan::DefaultSampler::LinearClamp)},
			.depthImageBinding {},
			.offsetBufferBinding {},
			.fixedRayCount {parameters.fixedRayCount},
			.relocationBackfaceThreshold {parameters.relocationBackfaceThreshold},
			.minFrontFaceDistance {parameters.minFrontFaceDistance},
			.shadowBias {parameters.depthBias},
			.hysteresis {parameters.hysteresis},
			.irradianceThreshold {parameters.irradianceThreshold},
			.lightToDarkThreshold {parameters.lightToDarkThreshold},
			.visualizerRadius {parameters.visualizerRadius},
			.enabled {parameters.enabled},
			.visualizeDepth {parameters.visualizeDepth},
			.visualizeDirections {parameters.visualizeDirections},
			.useMoments {parameters.useMoments},
			.relocationEnabled {parameters.relocationEnabled},
			.classificationEnabled {parameters.classificationEnabled},
	};
	update_spacing(volume);
	update_depth_texture(volume);
	update_irradiance_texture(volume);
	return add(volume);
}

const nyan::DDGIManager::DDGIVolumeParameters& nyan::DDGIManager::get_parameters(uint32_t id) const
{

	auto volumeView = r_registry.view<DDGIVolumeParameters>();
	for (auto [entity, parameters] : volumeView.each()) {
		if (parameters.ddgiVolume == id)
			return parameters;
	}
	assert(false && "Invalid Id");
	throw std::runtime_error("Invalid Id");
}

nyan::DDGIManager::DDGIVolumeParameters& nyan::DDGIManager::get_parameters(uint32_t id)
{

	auto volumeView = r_registry.view<DDGIVolumeParameters>();
	for (auto [entity, parameters] : volumeView.each()) {
		if (parameters.ddgiVolume == id)
			return parameters;
	}
	assert(false && "Invalid Id");
	throw std::runtime_error("Invalid Id");
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

void nyan::DDGIManager::set_max_ray_distance(uint32_t id, float maxRayDistance)
{
	auto& volume = DataManager<nyan::shaders::DDGIVolume>::get(id);
	volume.maxRayDistance = maxRayDistance;
}

const nyan::shaders::DDGIVolume& nyan::DDGIManager::get(uint32_t id) const
{
	return DataManager<nyan::shaders::DDGIVolume>::get(id);
}

void nyan::DDGIManager::update()
{

	auto volumeView = r_registry.view<DDGIVolumeParameters>();
	for (auto [entity, parameters] : volumeView.each()) {
		if (parameters.ddgiVolume == nyan::InvalidBinding) {
			parameters.ddgiVolume = add_ddgi_volume(parameters);
			parameters.dirty = true;
		}

		const auto& constDeviceVolume = DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume);

		if(	(constDeviceVolume.enabled != 0) != parameters.enabled ||
			constDeviceVolume.spacingX != parameters.spacing[0] ||
			constDeviceVolume.spacingY != parameters.spacing[1] ||
			constDeviceVolume.spacingZ != parameters.spacing[2] ||
			constDeviceVolume.gridOriginX != parameters.origin[0] ||
			constDeviceVolume.gridOriginY != parameters.origin[1] ||
			constDeviceVolume.gridOriginZ != parameters.origin[2] ||
			constDeviceVolume.probeCountX != parameters.probeCount[0] ||
			constDeviceVolume.probeCountY != parameters.probeCount[1] ||
			constDeviceVolume.probeCountZ != parameters.probeCount[2] ||
			constDeviceVolume.raysPerProbe != parameters.raysPerProbe ||
			constDeviceVolume.irradianceProbeSize != parameters.irradianceProbeSize ||
			constDeviceVolume.depthProbeSize != parameters.depthProbeSize ||
			constDeviceVolume.offsetBufferBinding == 0 || //Might not be ideal
			constDeviceVolume.fixedRayCount != parameters.fixedRayCount ||
			constDeviceVolume.relocationBackfaceThreshold != parameters.relocationBackfaceThreshold ||
			constDeviceVolume.minFrontFaceDistance != parameters.minFrontFaceDistance ||
			constDeviceVolume.shadowBias != parameters.depthBias ||
			constDeviceVolume.maxRayDistance != parameters.maxRayDistance ||
			constDeviceVolume.hysteresis != parameters.hysteresis ||
			constDeviceVolume.irradianceThreshold != parameters.irradianceThreshold ||
			constDeviceVolume.lightToDarkThreshold != parameters.lightToDarkThreshold ||
			constDeviceVolume.visualizerRadius != parameters.visualizerRadius ||
			(constDeviceVolume.visualizeDepth != 0) != parameters.visualizeDepth ||
			(constDeviceVolume.visualizeDirections != 0) != parameters.visualizeDirections ||
			(constDeviceVolume.useMoments != 0) != parameters.useMoments ||
			(constDeviceVolume.relocationEnabled != 0) != parameters.relocationEnabled ||
			(constDeviceVolume.classificationEnabled != 0) != parameters.classificationEnabled )
			parameters.dirty = true;

		if (!parameters.dirty)
			continue;


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
			.fixedRayCount {parameters.fixedRayCount},
			.relocationBackfaceThreshold {parameters.relocationBackfaceThreshold},
			.minFrontFaceDistance {parameters.minFrontFaceDistance},
			.shadowBias {parameters.depthBias},
			.maxRayDistance {parameters.maxRayDistance},
			.hysteresis {parameters.hysteresis},
			.irradianceThreshold {parameters.irradianceThreshold},
			.lightToDarkThreshold {parameters.lightToDarkThreshold},
			.visualizerRadius {parameters.visualizerRadius},
			.enabled {parameters.enabled},
			.visualizeDepth {parameters.visualizeDepth},
			.visualizeDirections {parameters.visualizeDirections},
			.useMoments {parameters.useMoments},
			.relocationEnabled {parameters.relocationEnabled},
			.classificationEnabled {parameters.classificationEnabled},
		};
		if (deviceVolume.fixedRayCount > deviceVolume.raysPerProbe)
			deviceVolume.fixedRayCount = deviceVolume.raysPerProbe;
		parameters.dirty = false;
		update_spacing(deviceVolume);
		update_depth_texture(deviceVolume);
		update_irradiance_texture(deviceVolume);
		update_offset_binding(parameters.ddgiVolume, deviceVolume);
		if (parameters.depthResource) {
			auto& depthResource = r_rendergraph.get_resource(parameters.depthResource);
			assert(depthResource.m_type == nyan::RenderResource::Type::Image);
			auto& imageAttachment = std::get< ImageAttachment>(depthResource.attachment);
			imageAttachment.width = static_cast<float>(deviceVolume.depthTextureSizeX);
			imageAttachment.height = static_cast<float>(deviceVolume.depthTextureSizeY);
		}
		else {
			parameters.depthResource = r_rendergraph.add_ressource(std::format("DDGI_Depth_{}", parameters.ddgiVolume), nyan::ImageAttachment{
				.format{VK_FORMAT_R16G16B16A16_SFLOAT},
				//.format{VK_FORMAT_R32G32B32A32_SFLOAT},
				.size{nyan::ImageAttachment::Size::Absolute},
				.width {static_cast<float>(deviceVolume.depthTextureSizeX)},
				.height {static_cast<float>(deviceVolume.depthTextureSizeY)}
				});
			for (const auto& read : m_reads) {
				auto& pass = r_rendergraph.get_pass(read);
				pass.add_read(parameters.depthResource);
			}
			for (const auto& [write, type] : m_writes) {
				auto& pass = r_rendergraph.get_pass(write);
				pass.add_write(parameters.depthResource, type);
			}
		}
		if (parameters.irradianceResource) {
			auto& irradianceResource = r_rendergraph.get_resource(parameters.irradianceResource);
			assert(irradianceResource.m_type == nyan::RenderResource::Type::Image);
			auto& imageAttachment = std::get< ImageAttachment>(irradianceResource.attachment);
			imageAttachment.width = static_cast<float>(deviceVolume.irradianceTextureSizeX);
			imageAttachment.height = static_cast<float>(deviceVolume.irradianceTextureSizeY);
		}
		else {
			parameters.irradianceResource = r_rendergraph.add_ressource(std::format("DDGI_Irradiance_{}", parameters.ddgiVolume), nyan::ImageAttachment{
				.format{VK_FORMAT_B10G11R11_UFLOAT_PACK32},
				//VK_FORMAT_E5B9G9R9_UFLOAT_PACK32
				//VK_FORMAT_R16G16B16_SFLOAT
				.size{nyan::ImageAttachment::Size::Absolute},
				.width {static_cast<float>(deviceVolume.irradianceTextureSizeX)},
				.height {static_cast<float>(deviceVolume.irradianceTextureSizeY)}
				});
			for (const auto& read : m_reads) {
				auto& pass = r_rendergraph.get_pass(read);
				pass.add_read(parameters.irradianceResource);
			}
			for (const auto& [write, type] : m_writes) {
				auto& pass = r_rendergraph.get_pass(write);
				pass.add_write(parameters.irradianceResource, type);
			}
		}
		// Update Probes -> Update Renderpass Read/Writes (if changed) -> Rendergraph update -> Update Bindings here
	}
}

void nyan::DDGIManager::begin_frame()
{
	//Update Bindings here
	auto volumeView = r_registry.view<DDGIVolumeParameters>();
	for (auto [entity, parameters] : volumeView.each()) {
		if (parameters.ddgiVolume == nyan::InvalidBinding) {
			continue;
		}
		const auto& constDeviceVolume = DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume);
		if (!m_writes.empty()) {
			auto& writePass = r_rendergraph.get_pass(m_writes.front().pass);
			auto depthImageBind = writePass.get_write_bind(parameters.depthResource, Renderpass::Write::Type::Compute);
			if (constDeviceVolume.depthImageBinding != depthImageBind)
				DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume).depthImageBinding = depthImageBind;
			auto irradianceImageBind = writePass.get_write_bind(parameters.irradianceResource, Renderpass::Write::Type::Compute);
			if (constDeviceVolume.irradianceImageBinding != irradianceImageBind)
				DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume).irradianceImageBinding = irradianceImageBind;
		}
		if (!m_reads.empty()) {
			auto& readPass = r_rendergraph.get_pass(m_reads.front());
			auto depthTextureBinding = readPass.get_read_bind(parameters.depthResource);
			if (constDeviceVolume.depthTextureBinding != depthTextureBinding)
				DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume).depthTextureBinding = depthTextureBinding;
			auto irradianceTextureBinding = readPass.get_read_bind(parameters.irradianceResource);
			if (constDeviceVolume.irradianceTextureBinding != irradianceTextureBinding)
				DataManager<nyan::shaders::DDGIVolume>::get(parameters.ddgiVolume).irradianceTextureBinding = irradianceTextureBinding;
		}
	}
}

void nyan::DDGIManager::end_frame()
{
}
void  nyan::DDGIManager::add_read(Renderpass::Id pass)
{
	m_reads.push_back(pass);
}
void  nyan::DDGIManager::add_write(Renderpass::Id pass, Renderpass::Write::Type type)
{
	m_writes.push_back({ pass, type });
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
	volume.depthTextureSizeY = volume.probeCountZ * volume.probeCountY * (2 + volume.depthProbeSize);
	volume.inverseDepthTextureSizeX = 1.0f / volume.depthTextureSizeX;
	volume.inverseDepthTextureSizeY = 1.0f / volume.depthTextureSizeY;
}

void nyan::DDGIManager::update_irradiance_texture(nyan::shaders::DDGIVolume& volume)
{
	//Add 1 pixel sample border
	volume.irradianceTextureSizeX = volume.probeCountX * (2 + volume.irradianceProbeSize);
	volume.irradianceTextureSizeY = volume.probeCountZ * volume.probeCountY * (2 + volume.irradianceProbeSize);
	volume.inverseIrradianceTextureSizeX = 1.0f / volume.irradianceTextureSizeX;
	volume.inverseIrradianceTextureSizeY = 1.0f / volume.irradianceTextureSizeY;
}

void nyan::DDGIManager::update_offset_binding(uint32_t volumeId, nyan::shaders::DDGIVolume& volume)
{
	if (m_offsets.size() <= volumeId)
		m_offsets.resize(volumeId + 1ull);	
	
	auto desiredSize = volume.probeCountX * volume.probeCountY * volume.probeCountZ;
	if (!m_offsets[volumeId] || desiredSize > m_offsets[volumeId]->counts) {
		vulkan::BufferInfo info{
			.size {desiredSize * sizeof(float) * 3},
			.usage {VK_BUFFER_USAGE_STORAGE_BUFFER_BIT},
			.memoryUsage {VMA_MEMORY_USAGE_GPU_ONLY},
		};
		uint32_t oldBinding{ ~0u };
		if (m_offsets[volumeId])
			oldBinding = m_offsets[volumeId]->binding;
		m_offsets[volumeId] = std::make_unique<Offsets>(r_device.create_buffer(info, {}, false), desiredSize);
		if (oldBinding == ~0)
			oldBinding = r_device.get_bindless_set().reserve_storage_buffer();
		m_offsets[volumeId]->binding = oldBinding;
		r_device.get_bindless_set().set_storage_buffer(oldBinding,
			VkDescriptorBufferInfo{ 
				.buffer {m_offsets[volumeId]->buffer->get_handle()},
				.offset {0},
				.range {info.size}
			});
	}
	volume.offsetBufferBinding = m_offsets[volumeId]->binding;
}
