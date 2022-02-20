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
		template<typename M, typename V, typename I>
		M* request_mesh(const std::string& name, const std::vector<V>& vertices, const std::vector<I>& indices) {
			if (auto res = m_staticMeshes.find(name); res != m_staticMeshes.end())
				return &res->second;

			M mesh{};
			if constexpr (std::is_same_v<V, StaticMesh::Vertex>) {
				mesh.tangentSpace = false;
			}
			else if constexpr (std::is_same_v < V, StaticMesh::TangentVertex>) {
				mesh.tangentSpace = true;
			}
			else {
				assert(false);
			}
			vulkan::BufferInfo buffInfo;
			buffInfo.size = vertices.size() * sizeof(V) + indices.size() * sizeof(I);
			buffInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			buffInfo.memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY;
			std::vector<vulkan::InputData> data;
			data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(vertices.data())), vertices.size() * sizeof(V) });
			data.push_back({ const_cast<void*>(reinterpret_cast<const void*>(indices.data())), indices.size() * sizeof(I) });
			auto vbo = r_device.create_buffer(buffInfo, data);

			m_usedBuffers.push_back(vbo);

			mesh.set_indices(vbo, static_cast<uint32_t>(vertices.size() * sizeof(V)), static_cast<uint32_t>(indices.size()));
			mesh.set_vertices(vbo, 0, static_cast<uint32_t>(vertices.size()));
			if constexpr (std::is_same_v < M, StaticMesh>) {
				auto res = m_staticMeshes.emplace(name, mesh);
				return &res.first->second;
			}
			else if constexpr (std::is_same_v < M, SkinnedMesh>) {
				//auto res = m_.emplace(name, mesh);
				//return &res.first->second;
				assert(false);
				return nullptr;
			}
			else {
				assert(false);
				return nullptr;
			}
		}/*
		StaticMesh* request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint16_t>& indices);
		StaticMesh* request_static_mesh(const std::string& name, const std::vector<StaticMesh::Vertex>& vertices, const std::vector<uint32_t>& indices);*/
	private:
		vulkan::LogicalDevice& r_device;
		std::vector<vulkan::BufferHandle> m_usedBuffers;
		std::unordered_map<std::string, StaticMesh> m_staticMeshes;

	};
}

#endif !RDMESHMANAGER_H