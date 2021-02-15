#pragma once
#ifndef RDRENDERER_H
#define RDRENDERER_H
#include "VkWrapper.h"
#include "ShaderManager.h"
#include "Material.h"
#include "Mesh.h"

namespace nyan {
	class Renderer {
	public:
		virtual void next_frame() = 0;
		virtual void end_frame() = 0;
	};
	//Use highest 32bit for material
	//lowest bit = tangentSpace
	//second bit = blendShapes
	//I.e. [Material] ... [Blendshape][TangentSpace]
	using RenderId = uint64_t;
	constexpr RenderId tangentSpaceBit	= 0b01ull;
	constexpr RenderId blendShapeBit	= 0b10ull;
	constexpr RenderId invalidId = ((~0ul) & ~tangentSpaceBit) & ~blendShapeBit;
	class VulkanRenderer;
	class RenderQueue {
		friend class VulkanRenderer;
	public:
		void clear();
	private:
		std::unordered_multimap<RenderId,StaticMesh*> m_staticMeshes;
		std::unordered_multimap<RenderId,SkinnedMesh*> m_skinnedMeshes;

	};
	struct RendererCamera {
		Math::mat44 view = Math::mat44::identity();
		Math::mat44 proj = Math::mat44::identity();
	};
	class VulkanRenderer : public Renderer {
	public:
		VulkanRenderer(vulkan::LogicalDevice& device, vulkan::ShaderManager* shaderManager);
		void queue_mesh(StaticMesh* mesh);
		void queue_mesh(SkinnedMesh* mesh);
		void update_camera(const RendererCamera& camera);
		void render(vulkan::CommandBufferHandle& cmd);
		void next_frame();
		void end_frame();
	private:
		vulkan::LogicalDevice& r_device;
		RenderQueue m_renderQueue;
		vulkan::ShaderManager* m_shaderManager = nullptr;
		vulkan::BufferHandle m_cameraBuffer;
	};
}

#endif !RDRENDERER_H