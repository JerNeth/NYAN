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
	std::memcpy(tmp, nyan::cubeVertices.data(), sizeof(nyan::cubeVertices));
	std::memcpy(tmp + sizeof(nyan::cubeVertices), nyan::cubeIndices.data(), sizeof(nyan::cubeIndices));
	auto vbo = r_device.create_buffer(buffInfo, tmp);

	m_usedBuffers.push_back(vbo);

	mesh.set_indices(vbo, sizeof(nyan::cubeVertices), nyan::cubeIndices.size());
	mesh.set_vertices(vbo, 0, nyan::cubeVertices.size());
	auto res = m_staticMeshes.emplace(name, mesh);
	
	return &res.first->second;
}