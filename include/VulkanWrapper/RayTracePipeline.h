#pragma once
#ifndef RAYTRACEPIPELINE_H
#define RAYTRACEPIPELINE_H

#include "VkWrapper.h"


namespace vulkan {
	class RTPipeline {
	public:
		RTPipeline(vulkan::LogicalDevice& device, const vulkan::RaytracingPipelineConfig& config);
		operator vulkan::PipelineId() const;
		const VkStridedDeviceAddressRegionKHR* rgen_region() const;
		const VkStridedDeviceAddressRegionKHR* miss_region() const;
		const VkStridedDeviceAddressRegionKHR* hit_region() const;
		const VkStridedDeviceAddressRegionKHR* callable_region() const;
	private:
		vulkan::PipelineId create_pipeline(const vulkan::RaytracingPipelineConfig& rayConfig);
		vulkan::BufferHandle create_sbt(const vulkan::RaytracingPipelineConfig& rayConfig);

		vulkan::LogicalDevice& r_device;
		vulkan::RaytracingPipelineConfig m_config;
		vulkan::PipelineId m_rtPipeline;
		vulkan::BufferHandle m_sbt;
		VkStridedDeviceAddressRegionKHR m_rgenRegion;
		VkStridedDeviceAddressRegionKHR m_missRegion;
		VkStridedDeviceAddressRegionKHR m_hitRegion;
		VkStridedDeviceAddressRegionKHR m_callableRegion;

	};
}


#endif !RAYTRACEPIPELINE_H