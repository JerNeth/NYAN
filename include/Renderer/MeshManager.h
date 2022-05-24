#pragma once
#ifndef RDMESHMANAGER_H
#define RDMESHMANAGER_H
#include "VkWrapper.h"
#include "Mesh.h"
#include "DataManager.h"
#include "MaterialManager.h"
#include "LinAlg.h"
namespace nyan {

	struct MeshData {
		std::string name;
		std::string material;
		Math::vec3 translate;
		Math::vec3 rotate;
		Math::vec3 scale;
		std::vector<uint32_t> indices;
		std::vector<Math::vec3> positions;
		std::vector<Math::vec2> uvs;
		std::vector<Math::vec3> normals;
		std::vector<Math::vec3> tangents;
	};
	using MeshID = uint32_t;
	using MeshInstanceID = uint32_t;
	using TransformID = uint32_t;

	using TransformBinding = Binding;
	struct TransformData {
		Math::Mat<float, 3, 4, true> transform;
		Math::Mat<float, 4, 4, true> view;
		Math::Mat<float, 4, 4, true> proj;
	};
	struct Transform {
		Math::vec3 position;
		Math::vec3 scale;
		Math::vec3 orientation;
		Math::Mat<float, 4, 4, true> view;
		Math::Mat<float, 4, 4, true> proj;
	};
	class MeshManager {
	private:
		struct Mesh {
			vulkan::BufferHandle buffer;
			VkDeviceSize offset;
			StaticTangentVulkanMesh mesh;
		};
	public:
		MeshManager(vulkan::LogicalDevice& device);
		MeshID add_mesh(const MeshData& data);
		MeshID get_mesh(const std::string& name);
		const StaticTangentVulkanMesh& get_static_tangent_mesh(MeshID idx);
		const StaticTangentVulkanMesh& get_static_tangent_mesh(const std::string& name);
	private:
		vulkan::LogicalDevice& r_device;
		std::unordered_map<MeshID, MeshManager::Mesh> m_staticTangentMeshes;
		std::unordered_map<std::string, MeshID> m_meshIndex;
		MeshID m_meshCounter {0};
	};
	struct MeshInstance {
		MeshID mesh_id;
		MaterialBinding material;
		uint32_t pad;
		//TODO change to transformId
		TransformData transform;
	};
	class MeshInstanceManager : public DataManager<MeshInstance, 1024 * 8> {
	public:
		MeshInstanceManager(vulkan::LogicalDevice& device);
		void set_transform(MeshInstanceID id, const TransformData& transform);
		MeshInstanceID add_instance(MeshID meshId, MaterialBinding material);
		MeshInstanceID add_instance(MeshID meshId, MaterialBinding material, const TransformData& transform);
	private:
		std::unordered_map<std::string, MeshID> m_meshIndex;
		MeshID m_instanceCounter{ 0 };
	};
}

#endif !RDMESHMANAGER_H