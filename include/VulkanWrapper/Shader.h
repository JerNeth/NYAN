#ifndef VKSHADER_H
#define VKSHADER_H
#pragma once
#include "VulkanIncludes.h"
#include <Util>
#include "LinAlg.h"

namespace vulkan {
	class LogicalDevice;
	class PipelineLayout;
	enum class DefaultSampler;
	struct ResourceBinding {
		union {
			VkDescriptorBufferInfo buffer;
			struct {
				VkDescriptorImageInfo fp;
				VkDescriptorImageInfo integer;
			} image;
			VkBufferView bufferView;
		};
		VkDeviceSize dynamicOffset;
	};	
	enum class ShaderStage : uint32_t{
		Vertex = Utility::bit_pos(VK_SHADER_STAGE_VERTEX_BIT),// 0,
		TesselationControl = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT), //1,
		TesselationEvaluation = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT), //2,
		Geometry = Utility::bit_pos(VK_SHADER_STAGE_GEOMETRY_BIT), //3,
		Fragment = Utility::bit_pos(VK_SHADER_STAGE_FRAGMENT_BIT), //4,
		Compute = Utility::bit_pos(VK_SHADER_STAGE_COMPUTE_BIT),//5,
		Size,
		Raygen = Utility::bit_pos(VK_SHADER_STAGE_RAYGEN_BIT_NV),//8
		AnyHit = Utility::bit_pos(VK_SHADER_STAGE_ANY_HIT_BIT_NV),//9
		ClosestHit = Utility::bit_pos(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV),//10
		Miss = Utility::bit_pos(VK_SHADER_STAGE_MISS_BIT_NV),//11
		Intersection = Utility::bit_pos(VK_SHADER_STAGE_INTERSECTION_BIT_NV),//12
		Callable = Utility::bit_pos(VK_SHADER_STAGE_CALLABLE_BIT_NV),//13
		
	};
	constexpr uint32_t NUM_SHADER_STAGES = static_cast<uint32_t>(ShaderStage::Size);

	struct DescriptorSetLayout {
		Utility::bitset<MAX_BINDINGS> imageSampler;
		Utility::bitset<MAX_BINDINGS> sampledBuffer;
		Utility::bitset<MAX_BINDINGS> storageImage;
		Utility::bitset<MAX_BINDINGS> uniformBuffer;
		Utility::bitset<MAX_BINDINGS> storageBuffer;
		Utility::bitset<MAX_BINDINGS> inputAttachment;
		Utility::bitset<MAX_BINDINGS> separateImage;
		Utility::bitset<MAX_BINDINGS> seperateSampler;
		Utility::bitset<MAX_BINDINGS> fp;
		Utility::bitset<MAX_BINDINGS> immutableSampler;
		Utility::bitarray<DefaultSampler, MAX_BINDINGS> immutableSamplers;
		std::array<uint8_t, MAX_BINDINGS> arraySizes;
		std::array<Utility::bitset<static_cast<size_t>(ShaderStage::Size), ShaderStage>, MAX_BINDINGS> stages{};
		//std::array<uint32_t, MAX_BINDINGS> stages{};
		friend bool operator==(DescriptorSetLayout& left, DescriptorSetLayout& right) {
			return left.imageSampler == right.imageSampler &&
				left.sampledBuffer == right.sampledBuffer &&
				left.storageImage == right.storageImage &&
				left.uniformBuffer == right.uniformBuffer &&
				left.storageBuffer == right.storageBuffer &&
				left.inputAttachment == right.inputAttachment &&
				left.separateImage == right.separateImage &&
				left.seperateSampler == right.seperateSampler &&
				left.fp == right.fp &&
				left.immutableSampler == right.immutableSampler &&
				left.arraySizes == right.arraySizes &&
				left.stages == right.stages;
		}
		friend bool operator==(const DescriptorSetLayout& left, const DescriptorSetLayout& right) {
			return left.imageSampler == right.imageSampler &&
				left.sampledBuffer == right.sampledBuffer &&
				left.storageImage == right.storageImage &&
				left.uniformBuffer == right.uniformBuffer &&
				left.storageBuffer == right.storageBuffer &&
				left.inputAttachment == right.inputAttachment &&
				left.separateImage == right.separateImage &&
				left.seperateSampler == right.seperateSampler &&
				left.fp == right.fp &&
				left.immutableSampler == right.immutableSampler &&
				left.arraySizes == right.arraySizes &&
				left.stages == right.stages;
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
			for (uint32_t descriptor = 0; descriptor < MAX_DESCRIPTOR_SETS; descriptor++) {
				assert((descriptors[descriptor].imageSampler & other.descriptors[descriptor].imageSampler).none());
				assert((descriptors[descriptor].sampledBuffer & other.descriptors[descriptor].sampledBuffer).none());
				assert((descriptors[descriptor].storageImage & other.descriptors[descriptor].storageImage).none());
				assert((descriptors[descriptor].uniformBuffer & other.descriptors[descriptor].uniformBuffer).none());
				assert((descriptors[descriptor].storageBuffer & other.descriptors[descriptor].storageBuffer).none());
				assert((descriptors[descriptor].inputAttachment & other.descriptors[descriptor].inputAttachment).none());
				assert((descriptors[descriptor].separateImage & other.descriptors[descriptor].separateImage).none());
				assert((descriptors[descriptor].seperateSampler & other.descriptors[descriptor].seperateSampler).none());
				assert((descriptors[descriptor].fp & other.descriptors[descriptor].fp).none());
				assert((descriptors[descriptor].immutableSampler & other.descriptors[descriptor].immutableSampler).none());
				descriptors[descriptor].imageSampler |= other.descriptors[descriptor].imageSampler;
				descriptors[descriptor].sampledBuffer |= other.descriptors[descriptor].sampledBuffer;
				descriptors[descriptor].storageImage |= other.descriptors[descriptor].storageImage;
				descriptors[descriptor].uniformBuffer |= other.descriptors[descriptor].uniformBuffer;
				descriptors[descriptor].storageBuffer |= other.descriptors[descriptor].storageBuffer;
				descriptors[descriptor].inputAttachment |= other.descriptors[descriptor].inputAttachment;
				descriptors[descriptor].separateImage |= other.descriptors[descriptor].separateImage;
				descriptors[descriptor].seperateSampler |= other.descriptors[descriptor].seperateSampler;
				descriptors[descriptor].fp |= other.descriptors[descriptor].fp;
				descriptors[descriptor].immutableSampler |= other.descriptors[descriptor].immutableSampler;
				for (uint32_t binding = 0; binding < MAX_BINDINGS; binding++) {
					if(other.descriptors[descriptor].immutableSampler.test(binding))
						descriptors[descriptor].immutableSamplers.set(other.descriptors[descriptor].immutableSamplers.get(binding), binding);
					assert(!(descriptors[descriptor].arraySizes[binding] != 0) || (other.descriptors[descriptor].arraySizes[binding] == 0));
					if(descriptors[descriptor].arraySizes[binding] == 0)
						descriptors[descriptor].arraySizes[binding] = other.descriptors[descriptor].arraySizes[binding];
					descriptors[descriptor].stages[binding] |= other.descriptors[descriptor].stages[binding];
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
	constexpr VkFormat get_format<float>() {
		return VK_FORMAT_R32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<uint8_t>() {
		return VK_FORMAT_R8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<int8_t>() {
		return VK_FORMAT_R8_UINT;
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
		return VK_FORMAT_R8G8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec2>() {
		return VK_FORMAT_R8G8_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::usvec2>() {
		return VK_FORMAT_R16G16_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::svec2>() {
		return VK_FORMAT_R16G16_SNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::vec2>() {
		return VK_FORMAT_R32G32_SFLOAT;
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
	constexpr VkFormat get_format<Math::vec3>() {
		return VK_FORMAT_R32G32B32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::ubvec3>() {
		return VK_FORMAT_R8G8B8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec3>() {
		return VK_FORMAT_R8G8B8_SNORM;
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
	constexpr VkFormat get_format<Math::vec4>() {
		return VK_FORMAT_R32G32B32A32_SFLOAT;
	}
	template< >
	constexpr VkFormat get_format<Math::ubvec4>() {
		return VK_FORMAT_R8G8B8A8_UNORM;
	}
	template< >
	constexpr VkFormat get_format<Math::bvec4>() {
		return VK_FORMAT_R8G8B8A8_SNORM;
	}
	template<typename T>
	constexpr VkFormat get_format(T t) {
		return get_format<T>();
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
	private:
		inline std::tuple<uint32_t, uint32_t, spirv_cross::SPIRType> get_values(const spirv_cross::Resource& resource, const spirv_cross::Compiler& comp) const;
		void inline array_info(std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>& layouts, const spirv_cross::SPIRType& type, uint32_t set, uint32_t binding) const;
		void create_module(const std::vector<uint32_t>& shaderCode);

		LogicalDevice& m_parent;
		VkShaderModule m_module = VK_NULL_HANDLE;
		ShaderStage m_stage;
		ShaderLayout m_layout;
		Utility::HashValue m_hashValue;
		
	}; 
	class Program {
	public:
		Program(const std::vector<Shader*>& shaders);
		Shader* get_shader(ShaderStage stage) const;
		void set_pipeline_layout(PipelineLayout* layout);
		PipelineLayout* get_pipeline_layout()const ;
		Utility::HashValue get_hash() const noexcept {
			return m_hashValue;
		}
	private:
		void set_shader(Shader* shader);
		PipelineLayout* m_layout = nullptr;
		std::array<Shader*, NUM_SHADER_STAGES> m_shaders{};
		Utility::HashValue m_hashValue;
	};
};

#endif //VKSHADER_H