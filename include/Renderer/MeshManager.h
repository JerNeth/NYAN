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
	using InstanceID = uint32_t;
	using TransformID = uint32_t;

	struct TransformBinding {
		union {
			struct {
				uint32_t binding;
				uint32_t id;
			};
			uint64_t data;
		};
		template<std::size_t Index>
		auto get() const
		{
			static_assert(Index < 2,
				"Index out of bounds for Custom::Binding");
			if constexpr (Index == 0) return id;
			if constexpr (Index == 1) return binding;
		}
	};
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
	union InstanceData {
		VkAccelerationStructureInstanceKHR instance;
		struct {
			Math::Mat<float, 3, 4, false> transformMatrix;
		};
	};
	class InstanceManager : public DataManager<InstanceData, TransformBinding, 1024 * 8> {
	public:
		InstanceManager(vulkan::LogicalDevice& device);
		void set_transform(TransformBinding id, const Math::Mat<float, 3, 4, false>& transformMatrix);
		void set_acceleration_structure(TransformBinding id, uint64_t accelerationStructureReference);
		void set_flags(TransformBinding id, VkGeometryInstanceFlagsKHR flags);
		void set_instance_shader_binding_table_record_offset(TransformBinding id, uint32_t instanceShaderBindingTableRecordOffset);
		void set_mask(TransformBinding id, uint32_t mask);
		void set_instance_custom_index(TransformBinding id, uint32_t instanceCustomIndex);
		void set_instance(TransformBinding id, const InstanceData& instance);
		TransformBinding add_instance(const InstanceData& instanceData = {.transformMatrix = Math::Mat<float, 3, 4, false>::identity()});
		std::pair<std::vector<uint32_t>, std::vector<VkDeviceAddress>> get_instance_data() const;
	private:
	};
}

#endif !RDMESHMANAGER_H