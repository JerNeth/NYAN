#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "Renderer.h"
#include "VulkanForwards.h"
#include "RayTracePipeline.h"
#include "RenderGraph.h"
#include <random>

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
			Math::vec4 randomRotation{ 0.4f, 0.6f, 0.8f, 1.f };
		};
		struct PipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t rayCount;
			VkBool32 filterIrradiance;
			static constexpr const char* rayCountShaderName{ "rayCount" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			friend bool operator==(const PipelineConfig& lhs, const PipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.rayCount == rhs.rayCount &&
					//lhs.probeSize == rhs.probeSize &&
					lhs.filterIrradiance == rhs.filterIrradiance;
			}
		};
		struct BorderPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			VkBool32 columns;
			VkBool32 filterIrradiance;
			static constexpr const char* columnsShaderName{ "columns" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			friend bool operator==(const BorderPipelineConfig& lhs, const BorderPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.columns == rhs.columns &&
					lhs.filterIrradiance == rhs.filterIrradiance;
			}
		};
		struct RelocatePipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			VkBool32 relocationEnabled;
			static constexpr const char* relocationEnabledShaderName{ "relocationEnabled" };
			friend bool operator==(const RelocatePipelineConfig& lhs, const RelocatePipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.relocationEnabled == rhs.relocationEnabled;
			}
		};
	public:
		DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void begin_frame();
	private:
		void render_volume(vulkan::RaytracingPipelineBind& bind, const PushConstants& constants, uint32_t numRays, uint32_t numProbes);
		void filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ);
		void copy_borders(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ);
		void relocate_probes(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		vulkan::PipelineId create_filter_pipeline(const PipelineConfig& config);
		vulkan::PipelineId create_border_pipeline(const BorderPipelineConfig& config);
		vulkan::PipelineId create_relocate_pipeline(const RelocatePipelineConfig& config);

		vulkan::RaytracingPipelineConfig generate_config();

		vulkan::RTPipeline m_rtPipeline;
		nyan::RenderResource::Id m_renderTarget;
		uint32_t m_borderSizeX{ 8 };
		uint32_t m_borderSizeY{ 8 };
		std::unordered_map<PipelineConfig, vulkan::PipelineId, Utility::Hash<PipelineConfig>> m_pipelines;
		std::unordered_map<BorderPipelineConfig, vulkan::PipelineId, Utility::Hash<BorderPipelineConfig>> m_borderPipelines;
		std::unordered_map<RelocatePipelineConfig, vulkan::PipelineId, Utility::Hash<RelocatePipelineConfig>> m_relocatePipelines;
		std::mt19937 m_generator{ 420 };
		std::uniform_real_distribution<float> m_dist {0.f, 1.f};
	};

	class DDGIVisualizer : Renderer {
		struct PushConstants
		{
			uint32_t sceneBinding;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
			Math::vec4 randomRotation{ 0.4f, 0.6f, 0.8f, 1.f };
		};
	public:
		DDGIVisualizer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting, const nyan::RenderResource::Id& depth);
	private:
		void visualize_volume(vulkan::GraphicsPipelineBind& bind, uint32_t volumeId);
		void create_pipeline();

		vulkan::PipelineId m_pipeline;
		bool m_enabled;
		Lighting m_lighting;
		nyan::RenderResource::Id m_depth;
		std::mt19937 m_generator{ 420 };
		std::uniform_real_distribution<float> m_dist{ 0.f, 1.f };
	};
}

#endif !RDDDGIRENDERER_H