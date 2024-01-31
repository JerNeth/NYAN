#pragma once
#ifndef RDMESH_H
#define RDMESH_H
import NYANMath;
#include <vector>
#include <string>
#include "Material.h"
#include "ShaderInterface.h"
namespace nyan {

	struct Mesh {
		enum class RenderType : uint32_t {
			Opaque = 0,
			AlphaTest = 1,
			AlphaBlend = 2,
		} type{ RenderType::Opaque };
		std::string name;
		std::string material;
		nyan::MaterialId materialBinding{ nyan::shaders::INVALID_BINDING };
		Math::vec3 translate;
		Math::vec3 rotate;
		Math::vec3 scale;
		std::vector<uint32_t> indices;
		std::vector<Math::vec3> positions;
		std::vector<Math::hvec2> uvs0;
		std::vector<Math::hvec2> uvs1;
		std::vector<Math::hvec2> uvs2;
		std::vector<Math::hvec3> normals;
		std::vector<Math::hvec4> tangents;
		std::vector<Math::unorm8> colors0;
	};
}

#endif //!RDMATERIAL_H