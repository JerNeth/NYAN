#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "VkWrapper.h"
#include "VulkanForwards.h"
#include "DataManager.h"
#include "RenderGraph.h"
#include "ShaderInterface.h"
#include "entt/fwd.hpp"

namespace nyan {
	class RenderManager;
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
			float depthBias{ 1e-2 };
		private:
			entt::hashed_string irradianceTexName;
			entt::hashed_string depthTexName;
			uint32_t ddgiVolume;
		};
	public:
		DDGIManager(vulkan::LogicalDevice& device, entt::registry& registry);
		uint32_t add_ddgi_volume(const DDGIVolumeParameters& parameters = {});
		void set_spacing(uint32_t id, const Math::vec3& spacing);
		void set_origin(uint32_t id, const Math::vec3& origin);
		void set_probe_count(uint32_t id, const Math::uvec3& probeCount);
		void set_irradiance_probe_size(uint32_t id, uint32_t probeSize);
		void set_depth_probe_size(uint32_t id, uint32_t probeSize);
		void set_rays_per_probe(uint32_t id, uint32_t rayCount);
		void set_depth_bias(uint32_t id, float depthBias);
		const nyan::shaders::DDGIVolume& get(uint32_t id) const;
		void update();
	private:
		void update_spacing(nyan::shaders::DDGIVolume& volume);
		void update_depth_texture(nyan::shaders::DDGIVolume& volume);
		void update_irradiance_texture(nyan::shaders::DDGIVolume& volume);

		entt::registry& r_registry;
	};

	class DDGIRenderer {
		struct PushConstants
		{
			uint32_t accBinding;
			uint32_t sceneBinding;
			uint32_t meshBinding;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
			Math::vec4 col{ 0.4f, 0.3f, 0.8f, 1.0f };
			Math::vec4 col2{ 0.4f, 0.6f, 0.8f, 1.f };
		};
	public:
		DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
	private:
		void render_volume(vulkan::ComputePipelineBind& bind, uint32_t volumeId);
		vulkan::PipelineId create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_renderDDGIPipeline;
		bool m_visualize;
	};

	class DDGIVisualizer {
		struct PushConstants
		{
			uint32_t sceneBinding;
			uint32_t meshBinding;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
		};
	public:
		DDGIVisualizer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
	private:
		void visualize_volume(vulkan::GraphicsPipelineBind& bind, uint32_t volumeId);
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_pipeline;
		bool m_enabled;
	};
}

#endif !RDDDGIRENDERER_H