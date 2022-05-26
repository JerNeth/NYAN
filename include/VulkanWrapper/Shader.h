#ifndef VKSHADER_H
#define VKSHADER_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <Util>
#include "LinAlg.h"

namespace vulkan {
	struct ResourceBinding {
		union {
			VkDescriptorBufferInfo buffer;
			VkDescriptorImageInfo image;
			VkBufferView bufferView; 
			VkAccelerationStructureKHR accelerationStructure;
		};
	};	
	enum class ShaderStage : uint32_t{
		Vertex = Utility::bit_pos(VK_SHADER_STAGE_VERTEX_BIT),// 0,
		TesselationControl = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT), //1,
		TesselationEvaluation = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT), //2,
		Geometry = Utility::bit_pos(VK_SHADER_STAGE_GEOMETRY_BIT), //3,
		Fragment = Utility::bit_pos(VK_SHADER_STAGE_FRAGMENT_BIT), //4,
		Compute = Utility::bit_pos(VK_SHADER_STAGE_COMPUTE_BIT),//5,
		Task = Utility::bit_pos(VK_SHADER_STAGE_TASK_BIT_NV), //6,
		Mesh = Utility::bit_pos(VK_SHADER_STAGE_MESH_BIT_NV), //7,
		Raygen = Utility::bit_pos(VK_SHADER_STAGE_RAYGEN_BIT_NV),//8
		AnyHit = Utility::bit_pos(VK_SHADER_STAGE_ANY_HIT_BIT_NV),//9
		ClosestHit = Utility::bit_pos(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV),//10
		Miss = Utility::bit_pos(VK_SHADER_STAGE_MISS_BIT_NV),//11
		Intersection = Utility::bit_pos(VK_SHADER_STAGE_INTERSECTION_BIT_NV),//12
		Callable = Utility::bit_pos(VK_SHADER_STAGE_CALLABLE_BIT_NV),//13
		Size,
	};
	constexpr uint32_t NUM_SHADER_STAGES = static_cast<uint32_t>(ShaderStage::Size);

	enum class DescriptorType : uint32_t {
		Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
		CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
		SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
		StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
		UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
		StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
		UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
		StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
		UniformDynamicBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
		StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
		InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		InlineUniformBlock = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
		AccelerationStructure = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
		Invalid
	};
	struct DescriptorLayout {
		uint32_t arraySize = 0;
		DescriptorType type = DescriptorType::Invalid;
		Utility::bitset <static_cast<size_t>(ShaderStage::Size), ShaderStage> stages {};
		friend bool operator==(const DescriptorLayout& left, const DescriptorLayout& right) {
			return left.type == right.type && 
				left.arraySize == right.arraySize &&
				left.stages == right.stages;
		}
	};
	struct DescriptorSetLayout {
		std::array<DescriptorLayout, MAX_BINDINGS> descriptors;
		friend bool operator==(const DescriptorSetLayout& left, const DescriptorSetLayout& right) {
			//if (left.descriptors.size() != right.descriptors.size())
			//	return false;
			for (size_t i = 0; i < left.descriptors.size(); i++)
				if (left.descriptors[i] != right.descriptors[i])
					return false;
			return true;
		}
	};

	struct ShaderLayout {
		std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptors;
		Utility::bitset<MAX_DESCRIPTOR_SETS> used;
		Utility::bitset<MAX_VERTEX_ATTRIBUTES> inputs;
		Utility::bitset<MAX_VERTEX_ATTRIBUTES> outputs;
		std::array<uint8_t, MAX_VERTEX_ATTRIBUTES> attributeElementCounts;
		VkPushConstantRange pushConstantRange{};
		friend bool operator==(ShaderLayout& left, ShaderLayout& right) {
			return left.descriptors == right.descriptors &&
				left.used == right.used &&
				left.inputs == right.inputs &&
				left.outputs == right.outputs &&
				left.attributeElementCounts == right.attributeElementCounts &&
				left.pushConstantRange.offset == right.pushConstantRange.offset &&
				left.pushConstantRange.size == right.pushConstantRange.size &&
				left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
		}
		friend bool operator==(const ShaderLayout& left, const ShaderLayout& right) {
			return left.descriptors == right.descriptors &&
				left.used == right.used &&
				left.inputs == right.inputs &&
				left.outputs == right.outputs &&
				left.attributeElementCounts == right.attributeElementCounts &&
				left.pushConstantRange.offset == right.pushConstantRange.offset &&
				left.pushConstantRange.size == right.pushConstantRange.size &&
				left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
		}
		void combine(const ShaderLayout& other) {
			used |= other.used;
			inputs |= other.inputs;
			outputs |= other.outputs;
			for (uint32_t i = 0; i < MAX_VERTEX_ATTRIBUTES; i++) {
				assert(!(attributeElementCounts[i] != 0) || (other.attributeElementCounts[i] == 0));
				if(attributeElementCounts[i] == 0)
					attributeElementCounts[i] = other.attributeElementCounts[i];
			}
			pushConstantRange.stageFlags |= other.pushConstantRange.stageFlags;
			pushConstantRange.size = Math::max(pushConstantRange.size, other.pushConstantRange.size);
			assert(pushConstantRange.size <= 128u);
			for (uint32_t descriptor = 0; descriptor < descriptors.size(); descriptor++) {
				auto& descriptorSetLayouts = descriptors[descriptor].descriptors;
				const auto& otherDescriptorSetLayouts = other.descriptors[descriptor].descriptors;

				assert(descriptorSetLayouts.size() == otherDescriptorSetLayouts.size());

				for (uint32_t binding = 0; binding < descriptorSetLayouts.size(); binding++) {
					auto& descriptorSetLayout = descriptorSetLayouts[binding];
					const auto& otherDescriptorSetLayout = otherDescriptorSetLayouts[binding];
					if (descriptorSetLayout.type == otherDescriptorSetLayout.type &&
						descriptorSetLayout.arraySize == otherDescriptorSetLayout.arraySize) {
						descriptorSetLayout.stages |= otherDescriptorSetLayout.stages;
					}
					else if(descriptorSetLayout.arraySize == 0) {
						descriptorSetLayout = otherDescriptorSetLayout;
					}
				}
			}
		}
	};

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
	constexpr VkFormat get_format(T t) {
		return get_format<T>();
	}

	template<typename T>
	constexpr size_t format_bytesize() {
		return format_bytesize(get_format<T>());
	}
	
	class Shader
	{
	public:
		Shader(LogicalDevice& parent, const std::vector<uint32_t>& shaderCode);
		~Shader();
		ShaderStage get_stage();
		void parse_shader(const std::vector<uint32_t>& shaderCode);
		const ShaderLayout& get_layout() const {
			return m_layout;
		}
		VkPipelineShaderStageCreateInfo get_create_info();
		Utility::HashValue get_hash();
		VkShaderModule get_module();
	private:
		void inline array_info(std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>& layouts, const spirv_cross::SPIRType& type, uint32_t set, uint32_t binding) const;
		void create_module(const std::vector<uint32_t>& shaderCode);

		LogicalDevice& m_parent;
		VkShaderModule m_module = VK_NULL_HANDLE;
		ShaderStage m_stage;
		ShaderLayout m_layout;
		Utility::HashValue m_hashValue;
		
	}; 
	class ShaderInstance {
	public:
		ShaderInstance(VkShaderModule module, VkShaderStageFlagBits stage);
		VkPipelineShaderStageCreateInfo get_stage_info() const;
		VkShaderStageFlagBits get_stage() const;
	private:
		VkShaderModule m_module;
		std::vector< VkSpecializationMapEntry> m_specialization;
		std::string m_entryPoint;
		VkSpecializationInfo m_specializationInfo;
		VkShaderStageFlagBits m_stage;
		std::vector<std::byte> m_dataStorage;
	};
	class ShaderStorage {
	public:
		ShaderStorage(LogicalDevice& device);

		ShaderId add_instance(ShaderId shaderId);
		ShaderId add_shader(const std::vector<uint32_t>& shaderCode);
		Shader* get_shader(ShaderId shaderId);
		ShaderInstance* get_instance(ShaderId instanceId);
		void clear();
	private:
		LogicalDevice& r_device;
		Utility::LinkedBucketList<ShaderInstance> m_instanceStorage;
		Utility::LinkedBucketList<Shader> m_shaderStorage;
	};

};

#endif //VKSHADER_H