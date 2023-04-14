#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "Renderer.h"
#include "VulkanForwards.h"
#include "RayTracePipeline.h"
#include "ddgi_restir_push_constants.h"
#include "ddgi_push_constants.h"
#include "RenderGraph.h"
#include <random>
#include <memory>

namespace nyan {


	class DDGIRenderer : public Renderer {
	private:
		struct PushConstantsDynamic
		{
			VkDeviceAddress targetAddress;
			uint32_t ddgiBinding;
			uint32_t ddgiCount;
			uint32_t ddgiIndex;
			uint32_t workBufferBinding;
			uint32_t numRows;
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
			VkBool32 dynamicRayAllocationEnabled;
			VkBool32 ReSTIREnabled;
			static constexpr const char* rayCountShaderName{ "maxRayCount" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			static constexpr const char* renderTargetImageFormatShaderName{ "renderTargetImageFormat" };
			static constexpr const char* imageFormatShaderName{ "imageFormat" };
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			static constexpr const char* dynamicRayAllocationEnabledShaderName{ "dynamicRayAllocationEnabled" };
			static constexpr const char* ReSTIREnabledShaderName{ "ReSTIREnabled" };
			friend bool operator==(const PipelineConfig& lhs, const PipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.rayCount == rhs.rayCount &&
					//lhs.probeSize == rhs.probeSize &&
					lhs.filterIrradiance == rhs.filterIrradiance &&
					lhs.renderTargetImageFormat == rhs.renderTargetImageFormat &&
					lhs.imageFormat == rhs.imageFormat &&
					lhs.dynamicRayAllocationEnabled == rhs.dynamicRayAllocationEnabled &&
					lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits;
			}
		};
		struct ReSTIRPipelineConfig {

		};
		struct BorderPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			VkBool32 columns;
			VkBool32 filterIrradiance;
			uint32_t imageFormat;
			uint32_t probeCountX;
			uint32_t probeCountY;
			uint32_t probeCountZ;
			uint32_t probeSize;
			uint32_t imageBinding;
			static constexpr const char* columnsShaderName{ "columns" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			static constexpr const char* imageFormatShaderName{ "imageFormat" };
			static constexpr const char* probeCountXShaderName{ "probeCountX" };
			static constexpr const char* probeCountYShaderName{ "probeCountY" };
			static constexpr const char* probeCountZShaderName{ "probeCountZ" };
			static constexpr const char* probeSizeShaderName{ "probeSize" };
			static constexpr const char* imageBindingShaderName{ "imageBinding" };
			friend bool operator==(const BorderPipelineConfig& lhs, const BorderPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.columns == rhs.columns &&
					lhs.filterIrradiance == rhs.filterIrradiance &&
					lhs.imageFormat == rhs.imageFormat &&
					lhs.probeCountX == rhs.probeCountX &&
					lhs.probeCountY == rhs.probeCountY &&
					lhs.probeCountZ == rhs.probeCountZ &&
					lhs.probeSize == rhs.probeSize &&
					lhs.imageBinding == rhs.imageBinding;
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
			uint32_t numRows;
			VkBool32 dynamicRayAllocationEnabled;
			VkBool32 ReSTIREnabled;
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			static constexpr const char* renderTargetImageFormatShaderName{ "renderTargetImageFormat" };
			static constexpr const char* dynamicRayAllocationEnabledShaderName{ "dynamicRayAllocationEnabled" };
			static constexpr const char* ReSTIREnabledShaderName{ "ReSTIREnabled" };
			static constexpr const char* numRowsShaderName{ "numRows" };
			friend bool operator==(const RTConfig& lhs, const RTConfig& rhs) {
				return lhs.renderTargetImageFormat == rhs.renderTargetImageFormat &&
					lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits &&
					lhs.dynamicRayAllocationEnabled == rhs.dynamicRayAllocationEnabled &&
					lhs.numRows == rhs.numRows;
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
		struct GatherPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t numRows;
			static constexpr const char* numRowsShaderName{ "numRows" };
			friend bool operator==(const GatherPipelineConfig& lhs, const GatherPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.numRows == rhs.numRows;
			}
		};
		struct PrefixSumPipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t subgroupSize;
			uint32_t numRows;
			uint32_t renderTargetImageWidthBits;
			static constexpr const char* subgroupSizeShaderName{ "subgroupSize" };
			static constexpr const char* numRowsShaderName{ "numRows" };
			static constexpr const char* renderTargetImageWidthBitsShaderName{ "renderTargetImageWidthBits" };
			friend bool operator==(const PrefixSumPipelineConfig& lhs, const PrefixSumPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.subgroupSize == rhs.subgroupSize &&
					lhs.numRows == rhs.numRows &&
					lhs.renderTargetImageWidthBits == rhs.renderTargetImageWidthBits;
			}
		};
	public:
		DDGIRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void begin_frame();
	private:
		void render(vulkan::CommandBuffer& cmd, nyan::Renderpass&);
		void render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const DDGIPushConstants& constants, uint32_t numRays, uint32_t numProbes);
		void render_volume(vulkan::RaytracingPipelineBind& bind, const vulkan::RTPipeline& pipeline, const DDGIPushConstants& constants, VkDeviceAddress address);
		void filter_volume(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants);
		void copy_borders(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants);
		void dispatch_compute(vulkan::ComputePipelineBind& bind, const DDGIPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ);
		void bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, const DDGIPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ);
		void relocate_probes(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIVolume& volume, const DDGIPushConstants& constants, uint32_t numFrames);
		void sum_variance(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void gather_variance(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		void prefix_sum(vulkan::ComputePipelineBind& bind, const PushConstantsDynamic& constants, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);
		vulkan::PipelineId create_filter_pipeline(const PipelineConfig& config);
		vulkan::PipelineId create_border_pipeline(const BorderPipelineConfig& config);
		vulkan::PipelineId create_relocate_pipeline(const RelocatePipelineConfig& config);
		vulkan::PipelineId create_scan_pipeline(const ScanPipelineConfig& config);
		vulkan::PipelineId create_gather_pipeline(const GatherPipelineConfig& config);
		vulkan::PipelineId create_prefix_sum_pipeline(const PrefixSumPipelineConfig& config);
		vulkan::RTPipeline& get_rt_pipeline(const RTConfig& config);

		vulkan::RaytracingPipelineConfig generate_config(const RTConfig& config);

		//vulkan::RTPipeline m_rtPipeline;
		nyan::RenderResource::Id m_renderTarget;
		uint32_t m_renderTargetWidthBits {10};
		uint32_t m_borderSizeX{ 8 };
		uint32_t m_borderSizeY{ 8 };
		std::unique_ptr<vulkan::BufferHandle> m_scratchBuffer;
		std::unique_ptr<vulkan::BufferHandle> dstBuf;
		std::unique_ptr<vulkan::BufferHandle> dstBuf2;
		uint32_t m_scratchBufferBinding{ ~0ul };
		uint32_t m_scanGroupSize{ 1024ul };
		uint32_t m_scanNumRows{ 16ul };
		uint32_t m_gatherGroupSize{ 64ul };
		uint32_t m_gatherNumRows{ 16ul };
		uint32_t m_prefixSumGroupSize{ 1024ul };
		uint32_t m_prefixSumNumRows{ 16ul };
		std::unordered_map<PipelineConfig, vulkan::PipelineId, Utility::Hash<PipelineConfig>> m_pipelines;
		std::unordered_map<BorderPipelineConfig, vulkan::PipelineId, Utility::Hash<BorderPipelineConfig>> m_borderPipelines;
		std::unordered_map<RelocatePipelineConfig, vulkan::PipelineId, Utility::Hash<RelocatePipelineConfig>> m_relocatePipelines;
		std::unordered_map<ScanPipelineConfig, vulkan::PipelineId, Utility::Hash<ScanPipelineConfig>> m_scanPipelines;
		std::unordered_map<GatherPipelineConfig, vulkan::PipelineId, Utility::Hash<GatherPipelineConfig>> m_gatherPipelines;
		std::unordered_map<PrefixSumPipelineConfig, vulkan::PipelineId, Utility::Hash<PrefixSumPipelineConfig>> m_prefixSumPipelines;
		std::unordered_map<RTConfig, std::unique_ptr<vulkan::RTPipeline>, Utility::Hash<RTConfig>> m_rtPipelines;
		std::unique_ptr<std::mt19937> m_generator{ new std::mt19937(420) };
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
		std::unique_ptr<std::mt19937> m_generator{ new std::mt19937(420) };
		std::uniform_real_distribution<float> m_dist{ 0.f, 1.f };
	};

	class DDGIReSTIRRenderer : public Renderer {
	private:
		struct PipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			uint32_t sampleCount;
			static constexpr const char* sampleCountShaderName{ "sampleCount" };
			friend bool operator==(const PipelineConfig& lhs, const PipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.sampleCount == rhs.sampleCount;
			}
		};
		struct ShadePipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			friend bool operator==(const ShadePipelineConfig& lhs, const ShadePipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ;
			}
		};
		struct SpatialResamplePipelineConfig
		{
			uint32_t workSizeX;
			uint32_t workSizeY;
			uint32_t workSizeZ;
			friend bool operator==(const SpatialResamplePipelineConfig& lhs, const SpatialResamplePipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ;
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
			uint32_t probeCountX;
			uint32_t probeCountY;
			uint32_t probeCountZ;
			uint32_t probeSize;
			uint32_t imageBinding;
			static constexpr const char* columnsShaderName{ "columns" };
			static constexpr const char* filterIrradianceShaderName{ "filterIrradiance" };
			static constexpr const char* imageFormatShaderName{ "imageFormat" };
			static constexpr const char* probeCountXShaderName{ "probeCountX" };
			static constexpr const char* probeCountYShaderName{ "probeCountY" };
			static constexpr const char* probeCountZShaderName{ "probeCountZ" };
			static constexpr const char* probeSizeShaderName{ "probeSize" };
			static constexpr const char* imageBindingShaderName{ "imageBinding" };
			friend bool operator==(const BorderPipelineConfig& lhs, const BorderPipelineConfig& rhs) {
				return lhs.workSizeX == rhs.workSizeX &&
					lhs.workSizeY == rhs.workSizeY &&
					lhs.workSizeZ == rhs.workSizeZ &&
					lhs.columns == rhs.columns &&
					lhs.filterIrradiance == rhs.filterIrradiance &&
					lhs.imageFormat == rhs.imageFormat &&
					lhs.probeCountX == rhs.probeCountX &&
					lhs.probeCountY == rhs.probeCountY &&
					lhs.probeCountZ == rhs.probeCountZ &&
					lhs.probeSize == rhs.probeSize &&
					lhs.imageBinding == rhs.imageBinding;
			}
		};
	public:
		DDGIReSTIRRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void begin_frame();
		void dump_to_disk();
		void clear_buffers();
	private:
		void render(vulkan::CommandBuffer& cmd, nyan::Renderpass&);
		vulkan::PipelineId create_filter_pipeline(const PipelineConfig& config);
		vulkan::PipelineId create_shade_pipeline(const ShadePipelineConfig& config);
		vulkan::PipelineId create_border_pipeline(const BorderPipelineConfig& config);
		vulkan::PipelineId create_spatial_reuse_pipeline(const SpatialResamplePipelineConfig& config);
		vulkan::RaytracingPipelineConfig generate_sample_generation_config();
		vulkan::RaytracingPipelineConfig generate_sample_validation_config();
		void generate_samples(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void validate_samples(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void resample(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void spatial_resample(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void copy_borders(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume);
		void shade(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void temporal_reuse(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void spatial_reuse(vulkan::CommandBuffer& cmd, const nyan::shaders::DDGIReSTIRVolume& volume, const DDGIReSTIRPushConstants& constants);
		void dispatch_compute(vulkan::ComputePipelineBind& bind, const DDGIReSTIRPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ);
		void bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, const DDGIReSTIRPushConstants& constants, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ);
		void bind_and_dispatch_compute(vulkan::CommandBuffer& cmd, vulkan::PipelineId pipelineId, uint32_t dispatchCountX, uint32_t dispatchCountY, uint32_t dispatchCountZ);


		std::unordered_map<PipelineConfig, vulkan::PipelineId, Utility::Hash<PipelineConfig>> m_filterPipelines;
		std::unordered_map<ShadePipelineConfig, vulkan::PipelineId, Utility::Hash<ShadePipelineConfig>> m_shadePipelines;
		std::unordered_map<BorderPipelineConfig, vulkan::PipelineId, Utility::Hash<BorderPipelineConfig>> m_borderPipelines;
		std::unordered_map<SpatialResamplePipelineConfig, vulkan::PipelineId, Utility::Hash<SpatialResamplePipelineConfig>> m_spatialResamplePipelines;
		std::unique_ptr<vulkan::RTPipeline> m_sampleGenerationPipeline;
		std::unique_ptr<vulkan::RTPipeline> m_sampleValidationPipeline;
		nyan::RenderResource::Id m_renderTarget;
		std::unique_ptr<std::mt19937> m_generator{new std::mt19937(420)};
		std::unique_ptr<vulkan::BufferHandle> m_temporalReservoirs;
		std::unique_ptr<vulkan::BufferHandle> m_screenshotBuffer;
		bool m_dirtyReservoirs{ false };
		bool m_screenshot{ false };
		int m_screenshotWidth{ 0 };
		int m_screenshotHeight{ 0 };
		bool m_dumpToDisk{ false };
		bool m_clear{ false };
		size_t m_currentReservoir{ 0 };
	};
}

#endif !RDDDGIRENDERER_H