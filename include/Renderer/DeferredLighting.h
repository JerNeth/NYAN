#pragma once
#ifndef RDDEFERREDLIGHTING_H
#define RDDEFERREDLIGHTING_H


#include "Renderer.h"
#include "RayTracePipeline.h"
#include "RenderGraph.h"

namespace nyan {
	class DeferredLighting : Renderer {
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
		DeferredLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
			nyan::RenderResource::Id albedoRead, nyan::RenderResource::Id normalRead, nyan::RenderResource::Id pbrRead, nyan::RenderResource::Id depthRead,
			nyan::RenderResource::Id stencilRead);
		void render(vulkan::GraphicsPipelineBind& bind);
	private:
		void create_pipeline();

		vulkan::PipelineId m_deferredPipeline;

		nyan::RenderResource::Id m_albedoRead;
		nyan::RenderResource::Id m_normalRead;
		nyan::RenderResource::Id m_pbrRead;
		nyan::RenderResource::Id m_depthRead;
		nyan::RenderResource::Id m_stencilRead;
	};

	class DeferredRayShadowsLighting : Renderer {
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
		DeferredRayShadowsLighting(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass,
			const GBuffer& gBuffer, const Lighting& lighting);
		void render(vulkan::RaytracingPipelineBind& bind);
	private:
		vulkan::RaytracingPipelineConfig generate_config();

		vulkan::RTPipeline m_pipeline;

		GBuffer m_gbuffer;

		Lighting m_lighting;
	};

	class LightComposite : Renderer{
		struct PushConstants {
			uint32_t specularBinding;
			uint32_t specularSampler;
			uint32_t diffuseBinding;
			uint32_t diffuseSampler;
		};
	public:
		LightComposite(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass
			, const Lighting& lighting);
		void render(vulkan::GraphicsPipelineBind& bind);
	private:
		void create_pipeline();

		vulkan::PipelineId m_compositePipeline;

		Lighting m_lighting;
	};
}
#endif !RDDEFERREDLIGHTING_H