#pragma once
#ifndef RDMESHRENDERER_H
#define RDMESHRENDERER_H

#include "VkWrapper.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "RenderGraph.h"
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
		uint32_t materialBinding;
		uint32_t materialId;
		uint32_t instanceBinding;
		uint32_t instanceId;
		uint32_t sceneBinding;
	};
	class MeshRenderer{
	public:
		MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass);
		void render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const MeshInstance& instance);
	private:
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		vulkan::ShaderManager& r_shaderManager;
		nyan::MeshManager& r_meshManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_staticTangentPipeline;
	};
	class RTMeshRenderer {
	public:
		RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass);
		void render(vulkan::RaytracingPipelineBind& bind);
	private:
		void create_pipeline();
		vulkan::LogicalDevice& r_device;
		entt::registry& r_registry;
		vulkan::ShaderManager& r_shaderManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_rtPipeline;
	};
}

#endif !RDMESHRENDERER_H