#include "Renderer/MeshManager.h"

using namespace vulkan;
using namespace nyan;

nyan::MeshManager::MeshManager(vulkan::LogicalDevice& device, bool buildAccelerationStructures) :
	r_device(device),
	m_builder(r_device),
	m_buildAccs(buildAccelerationStructures)
{
}

MeshID nyan::MeshManager::add_mesh(const MeshData& data)
{
	std::vector<vulkan::InputData> inputData;
	std::vector<uint32_t> offsets;
	inputData.push_back(InputData{ .ptr = data.indices.data(), .size = data.indices.size() * sizeof(uint32_t) });
	inputData.push_back(InputData{ .ptr = data.positions.data(), .size = data.positions.size() * sizeof(Math::vec3) });
	inputData.push_back(InputData{ .ptr = data.uvs.data(), .size = data.uvs.size() * sizeof(Math::vec2) });
	inputData.push_back(InputData{ .ptr = data.normals.data(), .size = data.normals.size() * sizeof(Math::vec3) });
	inputData.push_back(InputData{ .ptr = data.tangents.data(), .size = data.tangents.size() * sizeof(Math::vec3) });
	uint32_t offset = 0;
	for (const auto& inputDate : inputData) {
		offsets.push_back(offset);
		offset += inputDate.size;
	}
	vulkan::BufferInfo info{
		.size = offset,
		.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
				//| VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		.offset = 0,
		.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY

	};
	if (m_buildAccs) {
		info.usage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
			| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
			| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}
	MeshManager::Mesh mesh{
		.buffer = r_device.create_buffer(info, inputData),
		.offset {0},
		.mesh {
			.indexCount {static_cast<uint32_t>(data.indices.size())},
			.firstIndex {0},
			.vertexOffset {0},
			.firstInstance {0},

			.indexBuffer {mesh.buffer->get_handle()},
			.indexOffset {offsets[0]},
			.indexType {VK_INDEX_TYPE_UINT32},
			.positionBuffer {mesh.buffer->get_handle()},
			.texCoordBuffer {mesh.buffer->get_handle()},
			.normalBuffer {mesh.buffer->get_handle()},
			.tangentBuffer {mesh.buffer->get_handle()},


			.positionOffset {offsets[1]},
			.texCoordOffset {offsets[2]},
			.normalOffset {offsets[3]},
			.tangentOffset {offsets[4]},
		}
	};
	auto id = m_meshCounter++;
	if (m_buildAccs) {
		vulkan::AccelerationStructureBuilder::BLASInfo blasInfo{
			.vertexBuffer { mesh.mesh.positionBuffer },
			.vertexCount { static_cast<uint32_t>(data.positions.size()) },
			.vertexOffset { mesh.mesh.positionOffset },
			.vertexFormat { get_format<Math::vec3>() },
			.vertexStride { format_bytesize(get_format<Math::vec3>()) },
			.indexBuffer { mesh.mesh.indexBuffer },
			.indexOffset { mesh.mesh.indexOffset },
			.transformBuffer { VK_NULL_HANDLE },
			.transformOffset { 0 },
			.indexType { mesh.mesh.indexType },
		};
		auto ret = m_builder.queue_item(blasInfo);
		if(ret)
			m_pendingAccBuildIndex.emplace_back( *ret, id);
	}
	m_staticTangentMeshes.emplace(id, mesh);
	m_meshIndex.emplace(data.name, id);
	return id;
}

MeshID nyan::MeshManager::get_mesh(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	return m_meshIndex.find(name)->second;
}
void nyan::MeshManager::build()
{
	if (m_buildAccs) {
		auto handles = m_builder.build_pending();
		for (auto [handleId, meshId] : m_pendingAccBuildIndex) {
			assert(handleId < handles.size());
			auto it = m_staticTangentMeshes.find(meshId);
			assert(it != m_staticTangentMeshes.end());
			it->second.accStructure = handles[handleId];
		}
		m_pendingAccBuildIndex.clear();
	}
}
const StaticTangentVulkanMesh& nyan::MeshManager::get_static_tangent_mesh(MeshID idx)
{
	assert(m_staticTangentMeshes.find(idx) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(idx)->second.mesh;
}

const StaticTangentVulkanMesh& nyan::MeshManager::get_static_tangent_mesh(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	assert(m_staticTangentMeshes.find(m_meshIndex.find(name)->second) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(m_meshIndex.find(name)->second)->second.mesh;
}

std::optional<vulkan::AccelerationStructureHandle> nyan::MeshManager::get_acceleration_structure(MeshID idx)
{
	assert(m_staticTangentMeshes.find(idx) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(idx)->second.accStructure;
}

std::optional<vulkan::AccelerationStructureHandle> nyan::MeshManager::get_acceleration_structure(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	assert(m_staticTangentMeshes.find(m_meshIndex.find(name)->second) != m_staticTangentMeshes.end());
	return m_staticTangentMeshes.find(m_meshIndex.find(name)->second)->second.accStructure;
}

nyan::InstanceManager::InstanceManager(vulkan::LogicalDevice& device, bool buildAccelerationStructures) :
	DataManager(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR),
	m_builder(device),
	m_buildAccs(buildAccelerationStructures)
{
}

void nyan::InstanceManager::set_transform(InstanceId id, const Math::Mat<float, 3, 4, false>& transformMatrix)
{
	auto& instance = get(id);

	instance.transformMatrix = transformMatrix;

}
void nyan::InstanceManager::set_acceleration_structure(InstanceId id, uint64_t accelerationStructureReference)
{
	get(id).instance.accelerationStructureReference = accelerationStructureReference;
}
void nyan::InstanceManager::set_flags(InstanceId id, VkGeometryInstanceFlagsKHR flags)
{
	get(id).instance.flags = flags;
}
void nyan::InstanceManager::set_instance_shader_binding_table_record_offset(InstanceId id, uint32_t instanceShaderBindingTableRecordOffset)
{
	get(id).instance.instanceShaderBindingTableRecordOffset = instanceShaderBindingTableRecordOffset;
}
void nyan::InstanceManager::set_mask(InstanceId id, uint32_t mask)
{
	get(id).instance.mask = mask;
}
void nyan::InstanceManager::set_instance_custom_index(InstanceId id, uint32_t instanceCustomIndex)
{
	get(id).instance.instanceCustomIndex = instanceCustomIndex;
}
void nyan::InstanceManager::set_instance(InstanceId id, const InstanceData& instance)
{
	set(id, instance);
}
InstanceId nyan::InstanceManager::add_instance(const InstanceData& instanceData)
{
	return add(instanceData);
}

std::pair<uint32_t, VkDeviceAddress> nyan::InstanceManager::get_instance_data() const
{
	VkBufferDeviceAddressInfo addressInfo{
			.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
			.pNext = nullptr,
			.buffer = m_slot->buffer->get_handle()
	};
	return { m_slot->data.size(), vkGetBufferDeviceAddress(r_device, &addressInfo)};
}

void nyan::InstanceManager::build()
{
	if (m_buildAccs) {
		auto [count, address] = get_instance_data();
		m_tlas = m_builder.build_tlas(count, address);
	}
}

std::optional<vulkan::AccelerationStructureHandle> nyan::InstanceManager::get_tlas()
{
	return m_tlas;
}
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name)
//{
//	return nullptr;
//	//if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//	//	return &res->second;
//
//	//StaticMesh mesh;
//	//vulkan::BufferInfo buffInfo;
//	//buffInfo.size = sizeof(nyan::cubeVertices) + sizeof(nyan::cubeIndices);
//	//buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	//buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	//std::vector<vulkan::InputData> data;
//	//data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(nyan::cubeVertices.data())) ,sizeof(nyan::cubeVertices) });
//	//data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(nyan::cubeIndices.data())) ,  sizeof(nyan::cubeIndices) });
//	//auto vbo = r_device.create_buffer(buffInfo, data);
//
//	//m_usedBuffers.push_back(vbo);
//
//	//mesh.set_indices(vbo, sizeof(nyan::cubeVertices), static_cast<uint32_t>(nyan::cubeIndices.size()));
//	//mesh.set_vertices(vbo, 0, static_cast<uint32_t>(nyan::cubeVertices.size()));
//	//auto res = m_staticMeshes.emplace(name, mesh);
//
//	//return &res.first->second;
//}
//
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint16_t>& indices)
//{
//	if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//		return &res->second;
//
//	StaticMesh mesh;
//	vulkan::BufferInfo buffInfo;
//	buffInfo.size = vertices.size()* sizeof(StaticMesh::Vertex) + indices.size()*sizeof(uint16_t);
//	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	std::byte* tmp = (std::byte*)malloc(buffInfo.size);
//	std::memcpy(tmp, vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex));
//	std::memcpy(tmp + vertices.size() * sizeof(StaticMesh::Vertex), indices.data(), indices.size() * sizeof(uint16_t));
//	auto vbo = r_device.create_buffer(buffInfo, tmp);
//
//	m_usedBuffers.push_back(vbo);
//
//	mesh.set_indices(vbo, vertices.size() * sizeof(StaticMesh::Vertex), indices.size());
//	mesh.set_vertices(vbo, 0, vertices.size());
//	auto res = m_staticMeshes.emplace(name, mesh);
//
//	return &res.first->second;
//}
//StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint32_t>& indices)
//{
//	if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
//		return &res->second;
//
//	StaticMesh mesh;
//	vulkan::BufferInfo buffInfo;
//	buffInfo.size = vertices.size() * sizeof(StaticMesh::Vertex) + indices.size() * sizeof(uint32_t);
//	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
//	buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
//	std::byte* tmp = (std::byte*)malloc(buffInfo.size);
//	std::memcpy(tmp, vertices.data(), vertices.size() * sizeof(StaticMesh::Vertex));
//	std::memcpy(tmp + vertices.size() * sizeof(StaticMesh::Vertex), indices.data(), indices.size() * sizeof(uint32_t));
//	auto vbo = r_device.create_buffer(buffInfo, tmp);
//
//	m_usedBuffers.push_back(vbo);
//
//	mesh.set_indices(vbo, vertices.size() * sizeof(StaticMesh::Vertex), indices.size());
//	mesh.set_vertices(vbo, 0, vertices.size());
//	auto res = m_staticMeshes.emplace(name, mesh);
//
//	return &res.first->second;
//}
