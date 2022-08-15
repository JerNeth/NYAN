#pragma once
#ifndef RDDEFERREDLIGHTING_H
#define RDDEFERREDLIGHTING_H

#include "VkWrapper.h"
#include "RenderGraph.h"
#include "RenderManager.h"
#include "RayTracePipeline.h"
#include "entt/fwd.hpp"

namespace nyan {

	class DeferredLighting {
		struct PushConstants {
			uint32_t sceneBinding;
			uint32_t albedoBinding;
			uint32_t albedoSampler;
			uint32_t normalBinding;
			uint32_t normalSampler;
			uint32_t pbrBinding;
			uint32_t pbrSampler;
			uint32_t depthBinding;
			uint32_t depthSampler;
			uint32_t stencilBinding;
			uint32_t stencilSampler;
		};
	public:
		DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void render(vulkan::GraphicsPipelineBind& bind);
	private:
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_deferredPipeline;
	};

	class DeferredRayShadowsLighting {
		struct PushConstants {
			uint32_t accBinding;
			uint32_t sceneBinding;
			uint32_t albedoBinding;
			uint32_t albedoSampler;
			uint32_t normalBinding;
			uint32_t normalSampler;
			uint32_t pbrBinding;
			uint32_t pbrSampler;
			uint32_t depthBinding;
			uint32_t depthSampler;
			uint32_t stencilBinding;
			uint32_t stencilSampler;
			uint32_t diffuseImageBinding;
			uint32_t specularImageBinding;
		};
	public:
		DeferredRayShadowsLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void render(vulkan::RaytracingPipelineBind& bind);
	private:
		vulkan::RaytracingPipelineConfig generate_config();
		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::RTPipeline m_pipeline;
	};

	class LightComposite {
		struct PushConstants {
			uint32_t specularBinding;
			uint32_t specularSampler;
			uint32_t diffuseBinding;
			uint32_t diffuseSampler;
		};
	public:
		LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass);
		void render(vulkan::GraphicsPipelineBind& bind);
	private:
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		nyan::RenderManager& r_renderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_compositePipeline;
	};
}
#endif !RDDEFERREDLIGHTING_H