#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "VkWrapper.h"
#include "RenderGraph.h"
#include "RenderManager.h"
#include "ShaderInterface.h"
#include "entt/fwd.hpp"

namespace nyan {
	class DDGIManager : public DataManager<nyan::shaders::DDGIVolume> 
	{
	public:
		DDGIManager(vulkan::LogicalDevice& device);
		uint32_t add_ddgi_volume(const nyan::shaders::DDGIVolume& volume = {});
		void set_spacing(uint32_t id, const Math::vec3& spacing);
		void set_origin(uint32_t id, const Math::vec3& origin);
		void set_probe_count(uint32_t id, const Math::uvec3& probeCount);
		void set_irradiance_probe_size(uint32_t id, float probeSize);
		void set_depth_probe_size(uint32_t id, float probeSize);
		void set_rays_per_probe(uint32_t id, uint32_t rayCount);
		void set_depth_bias(uint32_t id, float depthBias);
		const nyan::shaders::DDGIVolume& get(uint32_t id) const;
	private:
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
		void render_volume(vulkan::RaytracingPipelineBind& bind, uint32_t volumeId);
		vulkan::RaytracingPipelineConfig generate_config();
		vulkan::PipelineId create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig);
		vulkan::BufferHandle create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig);

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_rtPipeline;
		vulkan::BufferHandle m_sbt;
		VkStridedDeviceAddressRegionKHR m_rgenRegion;
		VkStridedDeviceAddressRegionKHR m_hitRegion;
		VkStridedDeviceAddressRegionKHR m_missRegion;
		VkStridedDeviceAddressRegionKHR m_callableRegion;
	};
}

#endif !RDDDGIRENDERER_H