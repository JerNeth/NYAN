#pragma once
#ifndef RDMESHRENDERER_H
#define RDMESHRENDERER_H

#include "VkWrapper.h"
#include "ShaderManager.h"
#include "MeshManager.h"
#include "RenderGraph.h"

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
	class MeshRenderer{
	public:
		MeshRenderer(vulkan::LogicalDevice& device, vulkan::ShaderManager& shaderManager, nyan::MeshManager& meshManager, nyan::Renderpass& pass);
		void render(vulkan::GraphicsPipelineBind& bind, const MeshInstance& instance);
	private:
		void create_pipeline();

		vulkan::LogicalDevice& r_device;
		vulkan::ShaderManager r_shaderManager;
		nyan::MeshManager& r_meshManager;
		nyan::Renderpass& r_pass;
		vulkan::PipelineId m_staticTangentPipeline;
	};
}

#endif !RDMESHRENDERER_H