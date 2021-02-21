#pragma once
#ifndef RDMESHMANAGER_H
#define RDMESHMANAGER_H
#include "VkWrapper.h"
#include "Mesh.h"
#include "LinAlg.h"
namespace nyan {
	class StaticMesh;
	class MeshManager {
	public:
		MeshManager(vulkan::LogicalDevice& device);
		StaticMesh* request_static_mesh(const std::string& name);
		StaticMesh* request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint16_t>& indices);
		StaticMesh* request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint32_t>& indices);
	private:
		vulkan::LogicalDevice& r_device;
		std::vector<vulkan::BufferHandle> m_usedBuffers;
		std::unordered_map<std::string, StaticMesh> m_staticMeshes;

	};
}

#endif !RDMESHMANAGER_H