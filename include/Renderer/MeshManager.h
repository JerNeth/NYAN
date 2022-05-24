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

	struct TransformData {
		Math::Mat<float, 3, 4, false> transform;
		Math::Mat<float, 4, 4, true> view;
		Math::Mat<float, 4, 4, true> proj;
	};
	struct Transform {
		Math::vec3 position;
		Math::vec3 scale;
		Math::vec3 orientation;
	};
	class MeshManager {
	private:
		struct Mesh {
			vulkan::BufferHandle buffer;
			VkDeviceSize offset;
			std::optional<vulkan::AccelerationStructureHandle> accStructure;
			StaticTangentVulkanMesh mesh;
		};
	public:
		MeshManager(vulkan::LogicalDevice& device, bool buildAccelerationStructures = false);
		MeshID add_mesh(const MeshData& data);
		MeshID get_mesh(const std::string& name);
		void build();
		const StaticTangentVulkanMesh& get_static_tangent_mesh(MeshID idx);
		const StaticTangentVulkanMesh& get_static_tangent_mesh(const std::string& name);
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(MeshID idx);
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(const std::string& name);
	private:
		vulkan::LogicalDevice& r_device;
		vulkan::AccelerationStructureBuilder m_builder;
		bool m_buildAccs;
		std::unordered_map<MeshID, MeshManager::Mesh> m_staticTangentMeshes;
		std::unordered_map<std::string, MeshID> m_meshIndex;
		std::vector<std::pair<size_t, MeshID>> m_pendingAccBuildIndex;
		MeshID m_meshCounter {0};
	};
	union InstanceData {
		VkAccelerationStructureInstanceKHR instance;
		struct {
			Math::Mat<float, 3, 4, false> transformMatrix;
		};
	};
	struct InstanceId {
		uint32_t id;
		InstanceId(uint32_t id) : id(id) {}
		operator uint32_t() const {
			return id;
		}
	};
	class InstanceManager : public DataManager<InstanceData> {
	public:
		InstanceManager(vulkan::LogicalDevice& device, bool buildAccelerationStructures = false);
		void set_transform(InstanceId id, const Math::Mat<float, 3, 4, false>& transformMatrix);
		void set_acceleration_structure(InstanceId id, uint64_t accelerationStructureReference);
		void set_flags(InstanceId id, VkGeometryInstanceFlagsKHR flags);
		void set_instance_shader_binding_table_record_offset(InstanceId id, uint32_t instanceShaderBindingTableRecordOffset);
		void set_mask(InstanceId id, uint32_t mask);
		void set_instance_custom_index(InstanceId id, uint32_t instanceCustomIndex);
		void set_instance(InstanceId id, const InstanceData& instance);
		InstanceId add_instance(const InstanceData& instanceData = {.transformMatrix = Math::Mat<float, 3, 4, false>::identity()});
		std::pair<uint32_t, VkDeviceAddress> get_instance_data() const;
		void build();
		std::optional<vulkan::AccelerationStructureHandle> get_tlas();
	private:
		vulkan::AccelerationStructureBuilder m_builder;
		bool m_buildAccs;
		std::optional<vulkan::AccelerationStructureHandle> m_tlas;
	};
}

#endif !RDMESHMANAGER_H