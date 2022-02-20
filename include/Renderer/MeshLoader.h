#pragma once
#ifndef MESHLOADER_H
#define MESHLOADER_H
#include "Renderer/Mesh.h"
#include <variant>

namespace nyan {
	struct MeshData {
		enum class Type {
			Static,
			Skinned,
			StaticTangentSpace,
			SkinnedTangentSpace
		} type = Type::Static;
		std::string name;
		std::variant<
			std::vector<nyan::StaticMesh::Vertex>,
			std::vector<nyan::SkinnedMesh::Vertex>,
			std::vector<nyan::StaticMesh::TangentVertex>,
			std::vector<nyan::SkinnedMesh::TangentVertex>> vertices;
		std::variant<
			std::vector<uint32_t>,
			std::vector<uint16_t>> indices;
		std::vector<nyan::StaticMesh::Vertex>& get_static_vertices() {
			return std::get<std::vector<nyan::StaticMesh::Vertex>>(vertices);
		}
		std::vector<nyan::StaticMesh::TangentVertex>& get_static_tangent_vertices() {
			return std::get<std::vector<nyan::StaticMesh::TangentVertex>>(vertices);
		}
		std::vector<nyan::SkinnedMesh::Vertex>& get_skinned_vertices() {
			return std::get<std::vector<nyan::SkinnedMesh::Vertex>>(vertices);
		}
		std::vector<nyan::SkinnedMesh::TangentVertex>& get_skinned_tangent_vertices() {
			return std::get<std::vector<nyan::SkinnedMesh::TangentVertex>>(vertices);
		}
		std::vector<uint32_t>& get_indices32() {
			return std::get<std::vector<uint32_t>>(indices);
		}
		std::vector<uint16_t>& get_indices16() {
			return std::get<std::vector<uint16_t>>(indices);
		}
	};
	class MeshLoader {
	public:
	private:
	};

}
#endif 