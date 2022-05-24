#include "Renderer/MeshManager.h"

using namespace vulkan;
using namespace nyan;

nyan::MeshManager::MeshManager(vulkan::LogicalDevice& device) :
	r_device(device)
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
	MeshManager::Mesh mesh{
		.buffer = r_device.create_buffer(info, inputData),
		.offset = 0,
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
	m_staticTangentMeshes.emplace(id, mesh);
	m_meshIndex.emplace(data.name, id);
	return id;
}

MeshID nyan::MeshManager::get_mesh(const std::string& name)
{
	assert(m_meshIndex.find(name) != m_meshIndex.end());
	return m_meshIndex.find(name)->second;
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

nyan::InstanceManager::InstanceManager(vulkan::LogicalDevice& device) :
	DataManager(device, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT 
		| VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
		| VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)
{
}

void nyan::InstanceManager::set_transform(TransformBinding id, const Math::Mat<float, 3, 4, false>& transformMatrix)
{
	auto& instance = get(id);

	instance.transformMatrix = transformMatrix;

}
void nyan::InstanceManager::set_acceleration_structure(TransformBinding id, uint64_t accelerationStructureReference)
{
	get(id).instance.accelerationStructureReference = accelerationStructureReference;
}
void nyan::InstanceManager::set_flags(TransformBinding id, VkGeometryInstanceFlagsKHR flags)
{
	get(id).instance.flags = flags;
}
void nyan::InstanceManager::set_instance_shader_binding_table_record_offset(TransformBinding id, uint32_t instanceShaderBindingTableRecordOffset)
{
	get(id).instance.instanceShaderBindingTableRecordOffset = instanceShaderBindingTableRecordOffset;
}
void nyan::InstanceManager::set_mask(TransformBinding id, uint32_t mask)
{
	get(id).instance.mask = mask;
}
void nyan::InstanceManager::set_instance_custom_index(TransformBinding id, uint32_t instanceCustomIndex)
{
	get(id).instance.instanceCustomIndex = instanceCustomIndex;
}
void nyan::InstanceManager::set_instance(TransformBinding id, const InstanceData& instance)
{
	set(id, instance);
}
TransformBinding nyan::InstanceManager::add_instance(const InstanceData& instanceData)
{
	return add(instanceData);
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
