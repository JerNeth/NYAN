#include "Renderer/MeshManager.h"

using namespace vulkan;
using namespace nyan;

nyan::MeshManager::MeshManager(vulkan::LogicalDevice& device) :
	r_device(device)
{
}

StaticMesh* nyan::MeshManager::request_static_mesh(const std::string& name)
{
	
	if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
		return &res->second;

	StaticMesh mesh;
	vulkan::BufferInfo buffInfo;
	buffInfo.size = sizeof(nyan::cubeVertices) + sizeof(nyan::cubeIndices);
	buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
	std::byte* tmp = (std::byte*)malloc(sizeof(nyan::cubeVertices) + sizeof(nyan::cubeIndices));
	memcpy(tmp, nyan::cubeVertices.data(), sizeof(nyan::cubeVertices));
	memcpy(tmp + sizeof(nyan::cubeVertices), nyan::cubeIndices.data(), sizeof(nyan::cubeIndices));
	auto vbo = r_device.create_buffer(buffInfo, tmp);

	m_usedBuffers.push_back(vbo);

	mesh.set_indices(vbo, sizeof(nyan::cubeVertices), nyan::cubeIndices.size());
	mesh.set_vertices(vbo, 0, nyan::cubeVertices.size());
	auto res = m_staticMeshes.emplace(name, mesh);
	
	return &res.first->second;
}
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
