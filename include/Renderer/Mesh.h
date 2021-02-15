#pragma once
#ifndef RDMESH_H
#define RDMESH_H
#include "VkWrapper.h"
#include "LinAlg.h"
#include "Skeleton.h"
namespace nyan {

	class Material;
	struct TangentSpaceBasis {
		Math::ubvec3 normal;
		Math::ubvec3 bitangent;
		Math::ubvec3 tangent;
	};
	struct Transform {
		Math::mat44 transform;
		//std::array<float, 4 * 4> transform;
	};
	class Mesh {
	public:
		const Material* get_material() const noexcept {
			return material;
		}
		Material* get_material() noexcept {
			return material;
		}
		bool uses_tangent_space() const noexcept {
			return tangentSpace;
		}
		bool casts_shadows() const noexcept {
			return shadows;
		}
		void set_vertices(vulkan::Buffer* buffer, uint32_t offset, uint32_t count) noexcept {
			vertexBuffer = buffer;
			vertexOffset = offset;
			vertexCount = count;
			if (count < static_cast<uint16_t>(~0U))
				indexType = VK_INDEX_TYPE_UINT16;
			else
				indexType = VK_INDEX_TYPE_UINT32;
		}
		void set_indices(vulkan::Buffer* buffer, uint32_t offset, uint32_t count) noexcept {
			indexBuffer = buffer;
			indexOffset = offset;
			indexCount = count;
		}
		void set_transform(Transform* transform_) noexcept {
			transform = transform_;
		}
		void set_material(Material* material_) noexcept {
			material = material_;
		}
	protected:
		Transform* transform = nullptr;
		vulkan::Buffer* vertexBuffer = nullptr;
		uint32_t vertexOffset = 0;
		uint32_t vertexCount = 0;
		vulkan::Buffer* indexBuffer = nullptr;
		uint32_t indexOffset = 0;
		uint32_t indexCount = 0;
		VkIndexType indexType = VK_INDEX_TYPE_UINT32;
		Material* material = nullptr;
		bool tangentSpace = false;
		bool shadows = false;
	};
	class StaticMesh : public Mesh {
	public:
		//StaticMesh();
		struct Vertex {
			Math::vec3 pos;
			Math::usvec2 uv; //normalized [0,1] coordinates
			Math::ubvec4 color;
		}; 
		size_t vertex_count() const noexcept {
			return 0;
		}
		size_t vertex_stride() const noexcept {
			if (tangentSpace)
				return sizeof(Vertex) + sizeof(TangentSpaceBasis);
			else
				return sizeof(Vertex);
		}
		void render(vulkan::CommandBufferHandle& cmd);
	private:
	};
	
	class BlendShape {
		friend class SkinnedMesh;
	public:
		struct Vertex {
			Math::vec3 pos;
		};
		size_t vertex_count() const noexcept {
			return 0;
		}
		constexpr size_t vertex_stride() const noexcept {
			return sizeof(Vertex);
		}
	private:
		vulkan::Buffer* vertexBuffer = nullptr;
		uint32_t vertexOffset = 0;
	};
	class SkinnedMesh : public Mesh {
	public:
		struct Vertex {
			Math::vec3 pos;
			Math::usvec2 uv; //normalized [0,1] coordinates
			Math::ubvec4 boneIndices;
			Math::usvec4 boneWeights; //normalized [0,1] boneweights
		}; 
		size_t vertex_count() const noexcept {
			return 0;
		}
		size_t vertex_stride() const noexcept {
			if (tangentSpace)
				return sizeof(Vertex) + sizeof(TangentSpaceBasis);
			else
				return sizeof(Vertex);
		}
		bool has_blendshape() const noexcept {
			return blendshapeCount != 0;
		}

		void render(vulkan::CommandBufferHandle& cmd);
	private:
		Skeleton* skeleton = nullptr;
		BlendShape* blendshape = nullptr;
		uint32_t blendshapeCount = 0;
	};
	constexpr std::array<StaticMesh::Vertex, 8> vertices{
		StaticMesh::Vertex{Math::vec3({-0.5f, -0.5f, 0.0f}) , Math::unormVec<uint16_t>(Math::vec2({0.0f, 1.0f})), Math::unormVec<uint8_t>(Math::vec4({1.0f, 0.0f, 0.0f, 1.0f})) },
		StaticMesh::Vertex{Math::vec3({0.5f, -0.5f, 0.0f}) , Math::unormVec<uint16_t>(Math::vec2({1.0f, 1.0f})) , Math::unormVec<uint8_t>(Math::vec4({0.0f, 1.0f, 0.0f, 1.0f})) },
		StaticMesh::Vertex{Math::vec3({0.5f, 0.5f, 0.0f}) , Math::unormVec<uint16_t>(Math::vec2({1.0f, 0.0f})) , Math::unormVec<uint8_t>(Math::vec4({0.0f, 0.0f, 1.0f, 1.0f})) },
		StaticMesh::Vertex{Math::vec3({-0.5f, 0.5f, 0.0f}) , Math::unormVec<uint16_t>(Math::vec2({0.0f, 0.0f})) , Math::unormVec<uint8_t>(Math::vec4({1.0f, 1.0f, 1.0f, 1.0f})) },
		/*
		Vertex{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
		Vertex{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
		Vertex{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		Vertex{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
		*/
	};
	constexpr std::array<uint16_t, 6> indices = {
		0, 1, 2, 2, 3, 0,
		//4, 5, 6, 6, 7, 4
	};
}
#endif !RDMESH_H