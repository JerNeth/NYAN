#pragma once
#ifndef RDMESHMANAGER_H
#define RDMESHMANAGER_H
#include "VkWrapper.h"
#include "VulkanForwards.h"
#include "DataManager.h"
#include "MaterialManager.h"
#include "Mesh.h"
#include "ShaderInterface.h"
#include "Transform.h"
#include "AccelerationStructure.h"
namespace nyan {

	template<typename T>
	constexpr std::array<uint8_t, vulkan::MAX_VERTEX_INPUTS> get_formats() {
		return { static_cast<uint8_t>(vulkan::get_format<decltype(T::positions)::value_type>()),
		static_cast<uint8_t>(vulkan::get_format<decltype(T::uvs0)::value_type>()),
		static_cast<uint8_t>(vulkan::get_format<decltype(T::normals)::value_type>()),
		static_cast<uint8_t>(vulkan::get_format<decltype(T::tangents)::value_type>()) };
	}

	template<typename T>
	constexpr uint16_t get_num_formats() {
		return { 0u };
	}
	template<>
	constexpr uint16_t get_num_formats<nyan::Mesh>() {
		return { 4u };
	}
	using MeshID = uint32_t;

	struct StaticTangentVulkanMesh {
		uint32_t indexCount;
		uint32_t firstIndex;
		int32_t vertexOffset;
		uint32_t firstInstance;

		VkBuffer indexBuffer;
		VkDeviceSize indexOffset;
		VkIndexType indexType;
		struct VertexBuffers {
			VkBuffer positionBuffer;
			VkBuffer texCoordBuffer;
			VkBuffer normalBuffer;
			VkBuffer tangentBuffer;
			constexpr uint32_t size() const {
				return sizeof(VertexBuffers) / sizeof(VkDeviceSize);
			}
			constexpr const VkBuffer* data() const {
				return &positionBuffer;
			}
		} vertexBuffers;

		struct VertexOffsets {
			VkDeviceSize positionOffset;
			VkDeviceSize texCoordOffset;
			VkDeviceSize normalOffset;
			VkDeviceSize tangentOffset;
			constexpr uint32_t size() const {
				return sizeof(VertexOffsets) / sizeof(VkDeviceSize);
			}
			constexpr const VkDeviceSize* data() const {
				return &positionOffset;
			}
		} vertexOffsets;
		
	};
	class MeshManager : public DataManager<nyan::shaders::Mesh>{
	private:
		struct Mesh {
			vulkan::BufferHandle buffer;
			VkDeviceSize offset;
			std::optional<vulkan::AccelerationStructureHandle> accStructure;
			StaticTangentVulkanMesh mesh;
		};
	public:
		MeshManager(vulkan::LogicalDevice& device, nyan::MaterialManager& materialManager, bool buildAccelerationStructures = false);
		~MeshManager();
		MeshID add_mesh(const nyan::Mesh& data);
		MeshID get_mesh(const std::string& name);
		void build();
		const nyan::shaders::Mesh& get_shader_mesh(MeshID idx) const;
		const StaticTangentVulkanMesh& get_static_tangent_mesh(MeshID idx) const;
		const StaticTangentVulkanMesh& get_static_tangent_mesh(const std::string& name) const;
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(MeshID idx);
		std::optional<vulkan::AccelerationStructureHandle> get_acceleration_structure(const std::string& name);
	private:
		nyan::MaterialManager& r_materialManager;
		std::unique_ptr<vulkan::AccelerationStructureBuilder> m_builder;
		bool m_buildAccs;
		std::unordered_map<MeshID, MeshManager::Mesh> m_staticTangentMeshes;
		std::unordered_map<std::string, MeshID> m_meshIndex;
		std::vector<std::pair<size_t, MeshID>> m_pendingAccBuildIndex;
	};
	union InstanceData {
		VkAccelerationStructureInstanceKHR instance;
		struct Transform {
			Math::Mat<float, 3, 4, false> transformMatrix;
		} transform;
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
		const InstanceData& get_instance(InstanceId id) const;
		InstanceId add_instance(const InstanceData& instanceData = { .transform {.transformMatrix = Math::Mat<float, 3, 4, false>::identity()} });
		void build();
		std::optional<vulkan::AccelerationStructureHandle> get_tlas();
		std::optional<uint32_t> get_tlas_bind();
	private:
		std::unique_ptr<vulkan::AccelerationStructureBuilder> m_builder;
		bool m_buildAccs;
		std::optional<vulkan::AccelerationStructureHandle> m_tlas;
		uint32_t m_tlasBind;
	};


	class SceneManager : public DataManager<nyan::shaders::Scene>{
	public:
		SceneManager(vulkan::LogicalDevice& device);
		uint32_t add_point_light(const nyan::shaders::PointLight& light);
		void set_point_light(uint32_t id, const nyan::shaders::PointLight& light);
		void set_dirlight(const nyan::shaders::DirectionalLight& light);
		void set_sky_light(const nyan::shaders::SkyLight& light);
		void set_view_pos(const Math::vec3& pos);
		Math::vec3 get_view_pos() const;
		void set_camera_up(const Math::vec3& up);
		void set_view_matrix(const Math::Mat<float, 4, 4, true>& view);
		void set_view_matrix(const Math::Mat<float, 4, 4, true>& view, const Math::Mat<float, 4, 4, true>& viewInverse);
		void set_proj_matrix(const Math::Mat<float, 4, 4, true>& proj);
		void set_proj_matrix(const Math::Mat<float, 4, 4, true>& proj, const Math::Mat<float, 4, 4, true>& projInverse);
		//uint32_t get_binding() const;
		void update();
		//bool upload(vulkan::CommandBuffer& cmd);
	private:
		//vulkan::LogicalDevice& r_device;
		//vulkan::BufferHandle m_buffer;
		//uint32_t m_bind;
		//nyan::shaders::Scene m_sceneData;
		//bool m_dirtyScene;
	};
}

#endif !RDMESHMANAGER_H