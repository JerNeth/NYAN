#pragma once
#ifndef RDDEFERREDLIGHTING_H
#define RDDEFERREDLIGHTING_H

#include "VkWrapper.h"
#include "RenderGraph.h"
#include "RenderManager.h"
#include "entt/fwd.hpp"

namespace nyan {

	class DeferredLighting {
		struct PushConstants {
			uint32_t imgBinding;
			uint32_t imgSampler;
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
}
#endif !RDDEFERREDLIGHTING_H