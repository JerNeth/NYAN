#pragma once
#ifndef RDMESHRENDERER_H
#define RDMESHRENDERER_H

#include "VkWrapper.h"
#include "RenderGraph.h"
#include "RenderManager.h"
#include "entt/fwd.hpp"

namespace nyan {
	
	//class MeshRenderer : public Renderer {
	//public:
	//	void 
	//	//void render_geometry(vulkan::CommandBufferHandle& cmd);
	//	//void render(vulkan::CommandBufferHandle& cmd);
	//	virtual void next_frame() override;
	//	virtual void end_frame() override;
	//private:

	//};
	struct MeshInstance {
		uint32_t meshBinding;
		uint32_t instanceBinding;
		uint32_t instanceId;
		uint32_t sceneBinding;
	};
	class MeshRenderer{
	public:
		MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const MeshInstance& instance);
	private:
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_staticTangentPipeline;
	};
	class RTMeshRenderer {

		struct PushConstants
		{
			uint32_t imageBinding;
			uint32_t accBinding;
			uint32_t sceneBinding;
		};
	public:
		RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void render(vulkan::RaytracingPipelineBind& bind);
	private:
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

#endif !RDMESHRENDERER_H