#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "Renderer.h"
#include "VulkanForwards.h"
#include "RayTracePipeline.h"

namespace nyan {


	class DDGIRenderer : Renderer {
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
		vulkan::RaytracingPipelineConfig generate_config();

		vulkan::PipelineId m_renderDDGIPipeline;
		vulkan::RTPipeline m_rtPipeline;
		bool m_visualize;
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