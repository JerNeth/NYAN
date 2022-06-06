#pragma once
#ifndef RDMESH_H
#define RDMESH_H
#include "LinAlg.h"
#include <vector>
#include <string>
namespace nyan {

	struct Mesh {
		std::string name;
		std::string material;
		Math::vec3 translate;
		Math::vec3 rotate;
		Math::vec3 scale;
		std::vector<uint32_t> indices;
		std::vector<Math::vec3> positions;
		std::vector<Math::hvec2> uvs;
		std::vector<Math::hvec3> normals;
		std::vector<Math::hvec3> tangents;
	};
}

#endif !RDMATERIAL_H