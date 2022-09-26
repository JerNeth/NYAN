#pragma once
#ifndef RDDDGIMANAGER_H
#define RDDDGIMANAGER_H

#include "VkWrapper.h"
#include "VulkanForwards.h"
#include "DataManager.h"
#include "Rendergraph.h"
#include "ShaderInterface.h"
#include "entt/fwd.hpp"

namespace nyan {
	class DDGIManager : public DataManager<nyan::shaders::DDGIVolume>
	{
	public:
		struct DDGIVolumeParameters
		{
			friend class DDGIManager;
			Math::vec3 spacing{ 10.f, 10.f, 10.f };
			Math::vec3 origin{ 0.f, 0.f, 0.f };
			Math::uvec3 probeCount{ 16, 4, 16 };
			uint32_t raysPerProbe{ 128 };
			uint32_t irradianceProbeSize{ 8 };
			uint32_t depthProbeSize{ 16 };
			float depthBias{ 1e-2f };
			float maxRayDistance{5000.f};
			float hysteresis{ 9e-1f };
			float irradianceThreshold{0.25f};
			float lightToDarkThreshold{ 0.8f };
			float visualizerRadius{ 1.0f };
			RenderResource::Id depthResource{};
			RenderResource::Id irradianceResource{};
			uint32_t fixedRayCount{ 32 };
			uint32_t relocationBackfaceThreshold{ 16 };
			bool enabled{ true };
			bool visualization{ true };
			bool visualizeDepth{ false };
			bool visualizeDirections{ false };
			bool useMoments{ false };
			bool relocationEnabled{ false };
			bool classificationEnabled{ false };


			uint32_t ddgiVolume {nyan::InvalidBinding};
			bool dirty{ true };

		};
	private:
		struct Write {
			Renderpass::Id pass {};
			Renderpass::Write::Type type {};
		};
		struct Offsets {
			vulkan::BufferHandle buffer;
			uint32_t counts{ 0 };
		};
	public:
		DDGIManager(vulkan::LogicalDevice& device, nyan::Rendergraph& rendergraph, entt::registry& registry);
		uint32_t add_ddgi_volume(const DDGIVolumeParameters& parameters = {});
		const DDGIVolumeParameters& get_parameters(uint32_t id) const;
		DDGIVolumeParameters& get_parameters(uint32_t id);

		//Block probably unused for now
		void set_spacing(uint32_t id, const Math::vec3& spacing);
		void set_origin(uint32_t id, const Math::vec3& origin);
		void set_probe_count(uint32_t id, const Math::uvec3& probeCount);
		void set_irradiance_probe_size(uint32_t id, uint32_t probeSize);
		void set_depth_probe_size(uint32_t id, uint32_t probeSize);
		void set_rays_per_probe(uint32_t id, uint32_t rayCount);
		void set_depth_bias(uint32_t id, float depthBias);
		void set_max_ray_distance(uint32_t id, float maxRayDistance);
		const nyan::shaders::DDGIVolume& get(uint32_t id) const;

		void update();
		void begin_frame();
		void end_frame();
		void add_read(Renderpass::Id pass);
		void add_write(Renderpass::Id pass, Renderpass::Write::Type type);
	private:
		void update_spacing(nyan::shaders::DDGIVolume& volume);
		void update_depth_texture(nyan::shaders::DDGIVolume& volume);
		void update_irradiance_texture(nyan::shaders::DDGIVolume& volume);
		void update_offset_binding(uint32_t volumeId, nyan::shaders::DDGIVolume& volume);

		nyan::Rendergraph& r_rendergraph;
		entt::registry& r_registry;
		std::vector<Renderpass::Id> m_reads;
		std::vector<Write> m_writes;
		std::vector<std::unique_ptr<Offsets>> m_offsets;
	};
}

#endif !RDDDGIMANAGER_H