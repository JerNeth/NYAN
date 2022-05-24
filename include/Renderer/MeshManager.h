#pragma once
#ifndef RDMESHMANAGER_H
#define RDMESHMANAGER_H
#include "VkWrapper.h"
#include "Mesh.h"
#include "DataManager.h"
#include "MaterialManager.h"
#include "LinAlg.h"
namespace nyan {

	struct Mesh {
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
	struct MeshData {
		uint32_t materialBinding;
		uint32_t materialId;
		VkDeviceAddress indicesAddress;
		VkDeviceAddress uvAddress;
		VkDeviceAddress normalsAddress;
		VkDeviceAddress tangentsAddress;
	};
	class MeshManager : public DataManager<MeshData>{
	private:
		struct Mesh {
			vulkan::BufferHandle buffer;
			VkDeviceSize offset;
			std::optional<vulkan::AccelerationStructureHandle> accStructure;
			StaticTangentVulkanMesh mesh;
		};
	public:
		MeshManager(vulkan::LogicalDevice& device, nyan::MaterialManager& materialManager, bool buildAccelerationStructures = false);
		MeshID add_mesh(const nyan::Mesh& data);
		MeshID get_mesh(const std::string& name);
		void build();
		const StaticTangentVulkanMesh& get_static_tangent_mesh(MeshID idx);
		const StaticTangentVulkanMesh& get_static_tangent_mesh(const std::string& name);
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(MeshID idx);
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(const std::string& name);
	private:
		nyan::MaterialManager& r_materialManager;
		vulkan::AccelerationStructureBuilder m_builder;
		bool m_buildAccs;
		std::unordered_map<MeshID, MeshManager::Mesh> m_staticTangentMeshes;
		std::unordered_map<std::string, MeshID> m_meshIndex;
		std::vector<std::pair<size_t, MeshID>> m_pendingAccBuildIndex;
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
		void build();
		std::optional<vulkan::AccelerationStructureHandle> get_tlas();
		std::optional<uint32_t> get_tlas_bind();
	private:
		vulkan::AccelerationStructureBuilder m_builder;
		bool m_buildAccs;
		std::optional<vulkan::AccelerationStructureHandle> m_tlas;
		std::optional<uint32_t> m_tlasBind;
	};

	class SceneManager {
		struct SceneData {
			Math::Mat<float, 4, 4, true> view;
			Math::Mat<float, 4, 4, true> proj;
			Math::Mat<float, 4, 4, true> invView;
			Math::Mat<float, 4, 4, true> invProj;
		};
	public:
		SceneManager(vulkan::LogicalDevice& device);
		void set_view_matrix(const Math::Mat<float, 4, 4, true>& view);
		void set_proj_matrix(const Math::Mat<float, 4, 4, true>& proj);
		uint32_t get_binding() const;
		void update();
	private:
		vulkan::LogicalDevice& r_device;
		vulkan::BufferHandle m_buffer;
		uint32_t m_bind;
		SceneData m_sceneData;
		bool m_dirtyScene;
	};
}

#endif !RDMESHMANAGER_H