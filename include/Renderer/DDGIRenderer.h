#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "Renderer.h"
#include "VulkanForwards.h"
#include "RayTracePipeline.h"
#include "RenderGraph.h"
#include <random>
#include <memory>

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
			Math::vec4 randomRotation{ 0.4f, 0.6f, 0.8f, 1.f };
		};
		struct PushConstantsDynamic
		{
			VkDeviceAddress targetAddress;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
			uint32_t workBufferBinding;
		};
		struct PipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t rayCount;
			VkBool32 filterIrradiance;
			uint32_t renderTargetImageFormat;
			uint32_t imageFormat;
			uint32_t renderTargetImageWidthBits;
			static constexpr const char* rayCountShaderName{ "rayCount" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			static constexpr const char* renderTargetImageFormatShaderName{ "renderTargetImageFormat" };
			static constexpr const char* imageFormatShaderName{ "imageFormat" };
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			friend bool operator==(const PipelineConfig& lhs, const PipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.rayCount == rhs.rayCount &&
					//lhs.probeSize == rhs.probeSize &&
					lhs.filterIrradiance == rhs.filterIrradiance &&
					lhs.renderTargetImageFormat == rhs.renderTargetImageFormat &&
					lhs.imageFormat == rhs.imageFormat &&
					lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits;
			}
		};
		struct BorderPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			VkBool32 columns;
			VkBool32 filterIrradiance;
			uint32_t imageFormat;
			static constexpr const char* columnsShaderName{ "columns" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			static constexpr const char* imageFormatShaderName{ "imageFormat" };
			friend bool operator==(const BorderPipelineConfig& lhs, const BorderPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.columns == rhs.columns &&
					lhs.filterIrradiance == rhs.filterIrradiance &&
					lhs.imageFormat == rhs.imageFormat;
			}
		};
		struct RelocatePipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			VkBool32 relocationEnabled;
			uint32_t renderTargetImageWidthBits;
			uint32_t renderTargetImageFormat;
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			static constexpr const char* relocationEnabledShaderName{ "relocationEnabled" };
			static constexpr const char* renderTargetImageFormatShaderName{ "renderTargetImageFormat" };
			friend bool operator==(const RelocatePipelineConfig& lhs, const RelocatePipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.relocationEnabled == rhs.relocationEnabled &&
					lhs.renderTargetImageFormat == rhs.renderTargetImageFormat &&
					lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits;
			}
		};
		struct RTConfig
		{
			uint32_t renderTargetImageWidthBits;
			uint32_t renderTargetImageFormat;
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			static constexpr const char* renderTargetImageFormatShaderName{ "renderTargetImageFormat" };
			friend bool operator==(const RTConfig& lhs, const RTConfig& rhs) {
				return lhs.renderTargetImageFormat == rhs.renderTargetImageFormat &&
						lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits;
			}
		};
		struct ScanPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t subgroupSize;
			uint32_t numRows;
			static constexpr const char* subgroupSizeShaderName{ "subgroupSize" };
			static constexpr const char* numRowsShaderName{ "numRows" };
			friend bool operator==(const ScanPipelineConfig& lhs, const ScanPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.subgroupSize == rhs.subgroupSize &&
					lhs.numRows == rhs.numRows;
			}
		};
	public:
		DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void begin_frame();
	private:
		void render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const PushConstants& constants, uint32_t numRays, uint32_t numProbes);
		void render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const PushConstants& constants, VkDeviceAddress address);
		void filter_volume(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ);
		void copy_borders(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t probeCountX, uint32_t probeCountY, uint32_t probeCountZ);
		void relocate_probes(vulkan::ComputePipelineBind& bind, const PushConstants& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		vulkan::PipelineId create_filter_pipeline(const PipelineConfig& config);
		vulkan::PipelineId create_border_pipeline(const BorderPipelineConfig& config);
		vulkan::PipelineId create_relocate_pipeline(const RelocatePipelineConfig& config);
		vulkan::PipelineId create_scan_pipeline(const ScanPipelineConfig& config);
		vulkan::RTPipeline& get_rt_pipeline(const RTConfig& config);

		vulkan::RaytracingPipelineConfig generate_config(const RTConfig& config);

		//vulkan::RTPipeline m_rtPipeline;
		nyan::RenderResource::Id m_renderTarget;
		uint32_t m_renderTargetWidthBits {10};
		uint32_t m_borderSizeX{ 8 };
		uint32_t m_borderSizeY{ 8 };
		std::unique_ptr<vulkan::BufferHandle> m_scratchBuffer;
		uint32_t m_scratchBufferBinding{ ~0ul };
		uint32_t m_scanGroupSize{ 1024ul };
		uint32_t m_scanNumRows{ 16ul };
		std::unordered_map<PipelineConfig, vulkan::PipelineId, Utility::Hash<PipelineConfig>> m_pipelines;
		std::unordered_map<BorderPipelineConfig, vulkan::PipelineId, Utility::Hash<BorderPipelineConfig>> m_borderPipelines;
		std::unordered_map<RelocatePipelineConfig, vulkan::PipelineId, Utility::Hash<RelocatePipelineConfig>> m_relocatePipelines;
		std::unordered_map<ScanPipelineConfig, vulkan::PipelineId, Utility::Hash<ScanPipelineConfig>> m_scanPipelines;
		std::unordered_map<RTConfig, std::unique_ptr<vulkan::RTPipeline>, Utility::Hash<RTConfig>> m_rtPipelines;
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