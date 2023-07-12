#pragma once
#ifndef RDMESHRENDERER_H
#define RDMESHRENDERER_H

#include "Renderer.h"
#include "RayTracePipeline.h"
#include "RenderManager.h"
#include <random>
#include <memory>


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
			uint32_t diffuseImageBinding;
			uint32_t specularImageBinding;
			uint32_t rngSeed;
			uint32_t frameCount;
		};		
		struct RTConfig
		{
			uint32_t maxPathLength;
			VkBool32 antialiasing;
			static constexpr const char* maxPathLengthShaderName{ "maxPathLength" };
			static constexpr const char* antialiasingShaderName{ "antialiasing" };
			friend bool operator==(const RTConfig& lhs, const RTConfig& rhs) {
				return lhs.maxPathLength == rhs.maxPathLength &&
					lhs.antialiasing == rhs.antialiasing;
			}
		};
	public:
		RTMeshRenderer(vulkan::LogicalDevice& device, entt::registry& registry, nyan::RenderManager& renderManager, nyan::Renderpass& pass, const Lighting& lighting);
		void render(vulkan::CommandBuffer& cmd, nyan::Renderpass&);
		void reset();
		void set_antialiasing(bool antialising);
		void set_max_path_length(uint32_t maxPathLength);
	private:
		vulkan::RTPipeline& get_rt_pipeline(const RTConfig& config);
		vulkan::RaytracingPipelineConfig generate_rt_config(const RTConfig& config);
		std::unordered_map<RTConfig, std::unique_ptr<vulkan::RTPipeline>, Utility::Hash<RTConfig>> m_rtPipelines;
		Lighting m_lighting;
		std::unique_ptr<std::mt19937> m_generator{ new std::mt19937(420) };
		uint32_t m_frameCount{ 0 };
		uint32_t m_maxPathLength{ 10 };
		bool m_antialising{ true };
	};
}

#endif //!RDMESHRENDERER_H