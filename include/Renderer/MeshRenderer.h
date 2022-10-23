#pragma once
#ifndef RDMESHRENDERER_H
#define RDMESHRENDERER_H

#include "Renderer.h"
#include "RayTracePipeline.h"
#include "RenderManager.h"

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
	struct Deferred {};
	struct DeferredAlphaTest {};
	struct DeferredDoubleSided {};
	struct DeferredDoubleSidedAlphaTest {};
	struct Forward {};
	struct ForwardTransparent {};
	struct RayTraced {};
	class MeshRenderer : Renderer {
	private:
		struct PushConstants {
			uint32_t meshBinding;
			uint32_t instanceBinding;
			uint32_t instanceId;
			uint32_t sceneBinding;
		};
	public:
		MeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const GBuffer& gbuffer);
		void render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const PushConstants& instance);
		
	private:
		void create_pipeline();

		vulkan::PipelineId m_staticTangentPipeline;
		vulkan::PipelineId m_staticTangentAlphaDiscardPipeline;
		GBuffer m_gbuffer;
	};
	class ForwardMeshRenderer : Renderer {
	private:
		struct PushConstants {
			uint32_t meshBinding;
			uint32_t instanceBinding;
			uint32_t instanceId;
			uint32_t sceneBinding;
			uint32_t accBinding;
		};
	public:
		ForwardMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting, const nyan::RenderResource::Id& depth);
		void render(vulkan::GraphicsPipelineBind& bind, const MeshID& meshId, const PushConstants& instance);
	private:
		void create_pipeline();

		vulkan::PipelineId m_staticTangentPipeline;
		Lighting m_lighting;
		nyan::RenderResource::Id m_depth;
	};
	class RTMeshRenderer : Renderer {

		struct PushConstants
		{
			uint32_t accBinding;
			uint32_t sceneBinding;
			uint32_t meshBinding;
			uint32_t imageBinding;
			Math::vec4 col{ 0.4f, 0.3f, 0.8f, 1.0f };
			Math::vec4 col2{ 0.4f, 0.6f, 0.8f, 1.f };
		};
	public:
		RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, nyan::RenderResource::Id rendertarget);
		void render(vulkan::RaytracingPipelineBind& bind);
	private:
		vulkan::RaytracingPipelineConfig generate_config();

		vulkan::RTPipeline m_pipeline;
		nyan::RenderResource::Id m_rendertarget;
	};
}

#endif !RDMESHRENDERER_H