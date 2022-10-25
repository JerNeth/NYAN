#pragma once
#ifndef RDSHADERINTERFACE_H
#define RDSHADERINTERFACE_H

#include "LinAlg.h"
#define uint uint32_t
#define int int32_t
#define Indices VkDeviceAddress
#define Positions VkDeviceAddress
#define Uvs VkDeviceAddress
#define Normals VkDeviceAddress
#define Tangents VkDeviceAddress

#define ivec4 Math::ivec4
#define ivec3 Math::ivec3
#define ivec2 Math::ivec2
#define uvec4 Math::uvec4
#define uvec3 Math::uvec3
#define uvec2 Math::uvec2
#define vec4 Math::vec4
#define vec3 Math::vec3
#define vec2 Math::vec2
#define mat4x4 Math::mat44
#define mat3x3 Math::mat33
#define mat2x2 Math::mat22
#define mat4 Math::mat44
#define mat3 Math::mat33
#define mat2 Math::mat22
#define mat4x3 Math::mat34
#define mat3x4 Math::mat43
#define dmat4x4 Math::dmat44
#define dmat3x3 Math::dmat33
#define dmat2x2 Math::dmat22
#define dmat4 Math::dmat44
#define dmat3 Math::dmat33
#define dmat2 Math::dmat22
#define dmat4x3 Math::dmat34
#define dmat3x4 Math::dmat43

namespace nyan {
	namespace shaders {
		#include "structs.h"
		#include "formats.h"
		static_assert(INVALID_BINDING == ~0u);
	}
}

#undef uint
#undef int
#undef Indices
#undef Positions
#undef Uvs
#undef Normals
#undef Tangents
#undef ivec4
#undef ivec3
#undef ivec2
#undef uvec4
#undef uvec3
#undef uvec2
#undef vec4
#undef vec3
#undef vec2
#undef mat4x4
#undef mat3x3
#undef mat2x2
#undef mat4
#undef mat3
#undef mat2
#undef mat4x3
#undef mat3x4
#undef dmat4x4
#undef dmat3x3
#undef dmat2x2
#undef dmat4
#undef dmat3
#undef dmat2
#undef dmat4x3
#undef dmat3x4

#endif !RDSHADERINTERFACE_H