#ifndef VKMAXVALS_H
#define VKMAXVALS_H
#pragma once
#include "Math/LinAlg.h"

namespace vulkan {
	constexpr uint8_t MAX_BINDINGS = 16u;
	constexpr uint8_t MAX_DESCRIPTOR_SETS = 8u;
	constexpr uint8_t MAX_ATTACHMENTS = 8u;
	constexpr uint8_t MAX_SETS_PER_POOL = 16u;
	constexpr uint8_t PUSH_CONSTANT_SIZE = 128u;
	//32 on most 1.2 Device (64 on AMD) see maxVertexInputAttributes
	constexpr uint8_t MAX_VERTEX_ATTRIBUTES = 32u;
	constexpr uint8_t MAX_VERTEX_BINDINGS = 32u;
	constexpr uint8_t MAX_VERTEX_INPUTS = 16u;



	constexpr size_t format_bytesize(VkFormat format) {
		switch (format) {
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return 4;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
			return 1;
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
			return 2;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
			return 3;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
			return 4;
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
			return 2;
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
			return 4;
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
			return 6;
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
			return 8;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
			return 4;
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
			return 8;
		case VK_FORMAT_R32G32B32_SFLOAT:
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
			return 12;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
			return 16;
		case VK_FORMAT_R64_SFLOAT:
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
			return 8;
		case VK_FORMAT_R64G64_SFLOAT:
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
			return 16;
		case VK_FORMAT_R64G64B64_SFLOAT:
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
			return 24;
		case VK_FORMAT_R64G64B64A64_SFLOAT:
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
			return 32;
		default:
			return 0;
		}
	}
	constexpr size_t format_element_count(VkFormat format) {
		switch (format) {
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return 4;
		case VK_FORMAT_R8_UNORM:
		case VK_FORMAT_R8_SNORM:
		case VK_FORMAT_R8_UINT:
		case VK_FORMAT_R8_SINT:
		case VK_FORMAT_R8_USCALED:
		case VK_FORMAT_R8_SSCALED:
			return 1;
		case VK_FORMAT_R8G8_UNORM:
		case VK_FORMAT_R8G8_SNORM:
		case VK_FORMAT_R8G8_UINT:
		case VK_FORMAT_R8G8_SINT:
		case VK_FORMAT_R8G8_USCALED:
		case VK_FORMAT_R8G8_SSCALED:
			return 2;
		case VK_FORMAT_R8G8B8_UNORM:
		case VK_FORMAT_R8G8B8_SNORM:
		case VK_FORMAT_R8G8B8_UINT:
		case VK_FORMAT_R8G8B8_SINT:
		case VK_FORMAT_R8G8B8_USCALED:
		case VK_FORMAT_R8G8B8_SSCALED:
			return 3;
		case VK_FORMAT_R8G8B8A8_UNORM:
		case VK_FORMAT_R8G8B8A8_SNORM:
		case VK_FORMAT_R8G8B8A8_UINT:
		case VK_FORMAT_R8G8B8A8_SINT:
		case VK_FORMAT_R8G8B8A8_USCALED:
		case VK_FORMAT_R8G8B8A8_SSCALED:
			return 4;
		case VK_FORMAT_R16_UNORM:
		case VK_FORMAT_R16_SNORM:
		case VK_FORMAT_R16_UINT:
		case VK_FORMAT_R16_SINT:
		case VK_FORMAT_R16_SFLOAT:
		case VK_FORMAT_R16_USCALED:
		case VK_FORMAT_R16_SSCALED:
			return 1;
		case VK_FORMAT_R16G16_UNORM:
		case VK_FORMAT_R16G16_SNORM:
		case VK_FORMAT_R16G16_UINT:
		case VK_FORMAT_R16G16_SINT:
		case VK_FORMAT_R16G16_SFLOAT:
		case VK_FORMAT_R16G16_USCALED:
		case VK_FORMAT_R16G16_SSCALED:
			return 2;
		case VK_FORMAT_R16G16B16_UNORM:
		case VK_FORMAT_R16G16B16_SNORM:
		case VK_FORMAT_R16G16B16_UINT:
		case VK_FORMAT_R16G16B16_SINT:
		case VK_FORMAT_R16G16B16_SFLOAT:
		case VK_FORMAT_R16G16B16_USCALED:
		case VK_FORMAT_R16G16B16_SSCALED:
			return 3;
		case VK_FORMAT_R16G16B16A16_UNORM:
		case VK_FORMAT_R16G16B16A16_SNORM:
		case VK_FORMAT_R16G16B16A16_UINT:
		case VK_FORMAT_R16G16B16A16_SINT:
		case VK_FORMAT_R16G16B16A16_SFLOAT:
		case VK_FORMAT_R16G16B16A16_USCALED:
		case VK_FORMAT_R16G16B16A16_SSCALED:
			return 4;
		case VK_FORMAT_R32_SFLOAT:
		case VK_FORMAT_R32_UINT:
		case VK_FORMAT_R32_SINT:
			return 1;
		case VK_FORMAT_R32G32_SFLOAT:
		case VK_FORMAT_R32G32_UINT:
		case VK_FORMAT_R32G32_SINT:
			return 2;
		case VK_FORMAT_R32G32B32_SFLOAT:
		case VK_FORMAT_R32G32B32_UINT:
		case VK_FORMAT_R32G32B32_SINT:
			return 3;
		case VK_FORMAT_R32G32B32A32_SFLOAT:
		case VK_FORMAT_R32G32B32A32_UINT:
		case VK_FORMAT_R32G32B32A32_SINT:
			return 4;
		case VK_FORMAT_R64_SFLOAT:
		case VK_FORMAT_R64_UINT:
		case VK_FORMAT_R64_SINT:
			return 1;
		case VK_FORMAT_R64G64_SFLOAT:
		case VK_FORMAT_R64G64_UINT:
		case VK_FORMAT_R64G64_SINT:
			return 2;
		case VK_FORMAT_R64G64B64_SFLOAT:
		case VK_FORMAT_R64G64B64_UINT:
		case VK_FORMAT_R64G64B64_SINT:
			return 3;
		case VK_FORMAT_R64G64B64A64_SFLOAT:
		case VK_FORMAT_R64G64B64A64_UINT:
		case VK_FORMAT_R64G64B64A64_SINT:
			return 4;
		default:
			return 0;
		}
	}

	template<typename T>
	constexpr VkFormat get_format() {
		return VK_FORMAT_UNDEFINED;
	}
	template< >
	constexpr VkFormat get_format<uint32_t>() {
		return VK_FORMAT_R32_UINT;
	}
	template< >
	constexpr VkFormat get_format<int32_t>() {
		return VK_FORMAT_R32_SINT;
	}
	template< >
	constexpr VkFormat get_format<uint16_t>() {
		return VK_FORMAT_R16_UINT;
	}
	template< >
	constexpr VkFormat get_format<int16_t>() {
		return VK_FORMAT_R16_SINT;
	}
	template< >
	constexpr VkFormat get_format<::Math::half>() {
		return VK_FORMAT_R16_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<float>() {
		return VK_FORMAT_R32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<double>() {
		return VK_FORMAT_R64_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<uint8_t>() {
		return VK_FORMAT_R8_UINT;
	}
	template< >
	constexpr VkFormat get_format<int8_t>() {
		return VK_FORMAT_R8_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::unorm8>() {
		return VK_FORMAT_R8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::snorm8>() {
		return VK_FORMAT_R8_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::uvec2>() {
		return VK_FORMAT_R32G32_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::ivec2>() {
		return VK_FORMAT_R32G32_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::ubvec2>() {
		return VK_FORMAT_R8G8_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec2>() {
		return VK_FORMAT_R8G8_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::usvec2>() {
		return VK_FORMAT_R16G16_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::svec2>() {
		return VK_FORMAT_R16G16_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::un8vec2>() {
		return VK_FORMAT_R8G8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn8vec2>() {
		return VK_FORMAT_R8G8_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::un16vec2>() {
		return VK_FORMAT_R16G16_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn16vec2>() {
		return VK_FORMAT_R16G16_SNORM;
	}
	template< >
	constexpr VkFormat get_format<::Math::hvec2>() {
		return VK_FORMAT_R16G16_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::vec2>() {
		return VK_FORMAT_R32G32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::dvec2>() {
		return VK_FORMAT_R64G64_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::uvec3>() {
		return VK_FORMAT_R32G32B32_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::ivec3>() {
		return VK_FORMAT_R32G32B32_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::usvec3>() {
		return VK_FORMAT_R16G16B16_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::svec3>() {
		return VK_FORMAT_R16G16B16_SINT;
	}
	template< >
	constexpr VkFormat get_format<::Math::hvec3>() {
		return VK_FORMAT_R16G16B16_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::vec3>() {
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::dvec3>() {
		return VK_FORMAT_R64G64B64_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::ubvec3>() {
		return VK_FORMAT_R8G8B8_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec3>() {
		return VK_FORMAT_R8G8B8_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::un8vec3>() {
		return VK_FORMAT_R8G8B8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn8vec3>() {
		return VK_FORMAT_R8G8B8_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::un16vec3>() {
		return VK_FORMAT_R16G16B16_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn16vec3>() {
		return VK_FORMAT_R16G16B16_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::uvec4>() {
		return VK_FORMAT_R32G32B32A32_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::ivec4>() {
		return VK_FORMAT_R32G32B32A32_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::usvec4>() {
		return VK_FORMAT_R16G16B16A16_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::svec4>() {
		return VK_FORMAT_R16G16B16A16_SINT;
	}
	template< >
	constexpr VkFormat get_format<::Math::hvec4>() {
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::vec4>() {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::dvec4>() {
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::ubvec4>() {
		return VK_FORMAT_R8G8B8A8_UINT;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec4>() {
		return VK_FORMAT_R8G8B8A8_SINT;
	}
	template< >
	constexpr VkFormat get_format<Math::un8vec4>() {
		return VK_FORMAT_R8G8B8A8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn8vec4>() {
		return VK_FORMAT_R8G8B8A8_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::un16vec4>() {
		return VK_FORMAT_R16G16B16A16_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::sn16vec4>() {
		return VK_FORMAT_R16G16B16A16_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::quath>() {
		return VK_FORMAT_R16G16B16A16_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::quat>() {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::quatd>() {
		return VK_FORMAT_R64G64B64A64_SFLOAT;
	}
	template<typename T>
	constexpr VkFormat get_format([[maybe_unused]]T t) {
		return get_format<T>();
	}

	template<typename T>
	constexpr size_t format_bytesize() {
		return format_bytesize(get_format<T>());
	}
}
#endif