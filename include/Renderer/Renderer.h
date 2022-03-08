#pragma once
#ifndef RDRENDERER_H
#define RDRENDERER_H
#include "VkWrapper.h"
#include "ShaderManager.h"
#include "Material.h"
#include "Mesh.h"
#include <map>

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
	namespace GBuffer {
		constexpr uint32_t set = 0;
	}
	namespace Scene {
		constexpr uint32_t set = 1;
		constexpr uint32_t cameraBinding = 0;
		constexpr uint32_t materialBinding = 1;
		constexpr uint32_t textureBinding = 2;
		constexpr uint32_t transformBinding = 3;
		constexpr uint32_t lightBinding = 4;

		struct Material {
			int albedoTexture = 0;
			int normalTexture = 0;
			int metalTexture = 0;
			int roughnessTexture = 0;
			int displacementTexture = 0;
		};
		struct Transform {
			Math::mat43 transform;
		};
	}
	namespace Objects {
		constexpr uint32_t set = 2;
		constexpr uint32_t vertexBuffer = 0;
		constexpr uint32_t indexBuffer = 1;
	}


	constexpr RenderId tangentSpaceBit	= 0b01ull;
	constexpr RenderId blendShapeBit	= 0b10ull;
	constexpr RenderId invalidId = ((~0ul) & ~tangentSpaceBit) & ~blendShapeBit;
	class VulkanRenderer;
	class RenderQueue {
		friend class VulkanRenderer;
	public:
		void clear();
	private:
		std::multimap<RenderId,StaticMesh*> m_staticMeshes;
		std::multimap<RenderId,SkinnedMesh*> m_skinnedMeshes;

	};
	struct RendererCamera {
		Math::mat44 view = Math::mat44::identity();
		Math::mat44 proj = Math::mat44::identity();
	};
	struct Light {
		//Directional, Point, Spot
		//Directional:	vec3 dir
		//Point:		vec3 pos
		//Spot:			vec3 pos, vec3 dir, float angle
		//float attenuation;
		//float intensity
		//vec3 color
		//bool shadows
		//float shadowBias
		//
	};
	struct DirectionalLight {
		//Directional, Point, Spot
		//Directional:	vec3 dir
		//Point:		vec3 pos
		//Spot:			vec3 pos, vec3 dir, float angle
		//float attenuation;
		//float intensity
		//vec3 color
		//bool shadows
		//float shadowBias
		//
		Math::vec3 direction;
		float intensity;
		Math::vec3 color;
		bool castsShadows;

	};
	class VulkanRenderer : public Renderer {
	public:
		VulkanRenderer(vulkan::LogicalDevice& device, vulkan::ShaderManager* shaderManager);
		void queue_mesh(StaticMesh* mesh);
		void queue_mesh(SkinnedMesh* mesh);
		void add_light();
		void update_camera(const RendererCamera& camera);
		void render(vulkan::CommandBufferHandle& cmd);
		void next_frame() override;
		void end_frame() override;
	private:
		vulkan::LogicalDevice& r_device;
		RenderQueue m_renderQueue;
		vulkan::ShaderManager* m_shaderManager = nullptr;
		vulkan::BufferHandle m_cameraBuffer;
	};
}

#endif !RDRENDERER_H