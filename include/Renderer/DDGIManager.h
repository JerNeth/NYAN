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
			Math::vec3 spacing{ 1.f, 1.f, 1.f };
			Math::vec3 origin{ 0.f, 0.f, 0.f };
			Math::uvec3 probeCount{ 16, 16, 4 };
			uint32_t raysPerProbe{ 128 };
			uint32_t irradianceProbeSize{ 8 };
			uint32_t depthProbeSize{ 16 };
			float depthBias{ 1e-2f };
		private:
			uint32_t ddgiVolume {nyan::InvalidBinding};
			RenderResource::Id depthResource{};
			RenderResource::Id irradianceResource{};
			bool dirty{ true };

		};
	private:
		struct Write {
			Renderpass::Id pass {};
			Renderpass::Write::Type type {};
		};
	public:
		DDGIManager(vulkan::LogicalDevice& device, nyan::Rendergraph& rendergraph, entt::registry& registry);
		uint32_t add_ddgi_volume(const DDGIVolumeParameters& parameters = {});

		//Block probably unused for now
		void set_spacing(uint32_t id, const Math::vec3& spacing);
		void set_origin(uint32_t id, const Math::vec3& origin);
		void set_probe_count(uint32_t id, const Math::uvec3& probeCount);
		void set_irradiance_probe_size(uint32_t id, uint32_t probeSize);
		void set_depth_probe_size(uint32_t id, uint32_t probeSize);
		void set_rays_per_probe(uint32_t id, uint32_t rayCount);
		void set_depth_bias(uint32_t id, float depthBias);
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

		nyan::Rendergraph& r_rendergraph;
		entt::registry& r_registry;
		std::vector<Renderpass::Id> m_reads;
		std::vector<Write> m_writes;
	};
}

#endif !RDDDGIMANAGER_H