#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "Renderer.h"
#include "VulkanForwards.h"
#include "RayTracePipeline.h"
#include "RenderGraph.h"

namespace nyan {


	class DDGIRenderer : Renderer {
	private:
		struct PushConstants
		{
			uint32_t accBinding;
			uint32_t sceneBinding;
			uint32_t meshBinding;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
			uint32_t renderTarget;
			Math::vec4 col{ 0.4f, 0.3f, 0.8f, 1.0f };
			Math::vec4 col2{ 0.4f, 0.6f, 0.8f, 1.f };
		};
		struct PipelineConfig 
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t ray_count;
			static constexpr const char* ray_countShaderName{"ray_count"};
			friend bool operator==(const PipelineConfig& lhs, const PipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.ray_count == rhs.ray_count;
			}
		};
	public:
		DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void begin_frame();
	private:
		void render_volume(vulkan::RaytracingPipelineBind& bind, const PushConstants& constants, uint32_t numRays, uint32_t numProbes);
		void filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ);
		vulkan::PipelineId create_pipeline(const PipelineConfig& config);
		vulkan::RaytracingPipelineConfig generate_config();

		vulkan::RTPipeline m_rtPipeline;
		nyan::RenderResource::Id m_renderTarget;
		std::unordered_map<PipelineConfig, vulkan::PipelineId, Utility::Hash<PipelineConfig>> m_pipelines;
	};

	class DDGIVisualizer : Renderer {
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

		vulkan::PipelineId m_pipeline;
		bool m_enabled;
	};
}

#endif !RDDDGIRENDERER_H