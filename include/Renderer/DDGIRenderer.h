#pragma once
#ifndef RDDDGIRENDERER_H
#define RDDDGIRENDERER_H

#include "VkWrapper.h"
#include "RenderGraph.h"
#include "RenderManager.h"
#include "entt/fwd.hpp"

namespace nyan {
	struct DDGIStuff {
		//Assuming Grid
		Math::vec3 spacing;
		Math::vec3 gridOrigin;
		Math::uvec3 probeCount;
		uint32_t raysPerProbe;
		uint32_t irradianceProbeSize;
		uint32_t depthProbeSize;
		Math::vec2 irradianceTextureSize;
		uint32_t irradianceTextureBinding;
		uint32_t irradianceTextureSampler;
		Math::vec2 depthTextureSize;
		uint32_t depthTextureBinding;
		uint32_t depthTextureSampler;
		float shadowBias;
	};
	class DDGIRenderer {
	public:
		DDGIRenderer(vulkan::LogicalDevice& device);
	private:
		vulkan::LogicalDevice& r_device
	};
}

#endif !RDDDGIRENDERER_H