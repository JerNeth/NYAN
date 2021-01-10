#ifndef VKSHADER_H
#define VKSHADER_H
#pragma once
#include "VulkanIncludes.h"
#include <vector>
#include "DescriptorSet.h"
#include "Utility.h"

namespace Vulkan {
	class LogicalDevice;
	class PipelineLayout;
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
	struct ShaderLayout {
		std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptors;
		std::bitset<MAX_DESCRIPTOR_SETS> used;
		std::bitset<32> inputs;
		std::bitset<32> outputs;
		std::array<uint16_t, MAX_DESCRIPTOR_SETS> stagesForSets;
		VkPushConstantRange pushConstantRange{};
		friend bool operator==(ShaderLayout& left, ShaderLayout& right){
			return left.descriptors == right.descriptors &&
				left.used == right.used &&
				left.inputs == right.inputs &&
				left.outputs == right.outputs &&
				left.pushConstantRange.offset == right.pushConstantRange.offset &&
				left.pushConstantRange.size == right.pushConstantRange.size &&
				left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
		}
		friend bool operator==(const ShaderLayout& left, const ShaderLayout& right){
			return left.descriptors == right.descriptors &&
				left.used == right.used &&
				left.inputs == right.inputs &&
				left.outputs == right.outputs &&
				left.pushConstantRange.offset == right.pushConstantRange.offset &&
				left.pushConstantRange.size == right.pushConstantRange.size &&
				left.pushConstantRange.stageFlags == right.pushConstantRange.stageFlags;
		}
	};
	enum class ShaderStage {
		Vertex = Utility::bit_pos(VK_SHADER_STAGE_VERTEX_BIT),// 0,
		TesselationControl = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT), //1,
		TesselationEvaluation = Utility::bit_pos(VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT), //2,
		Geometry = Utility::bit_pos(VK_SHADER_STAGE_GEOMETRY_BIT), //3,
		Fragment = Utility::bit_pos(VK_SHADER_STAGE_FRAGMENT_BIT), //4,
		Compute = Utility::bit_pos(VK_SHADER_STAGE_COMPUTE_BIT),//5,
		Raygen = Utility::bit_pos(VK_SHADER_STAGE_RAYGEN_BIT_NV),//8
		AnyHit = Utility::bit_pos(VK_SHADER_STAGE_ANY_HIT_BIT_NV),//9
		ClosestHit = Utility::bit_pos(VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV),//10
		Miss = Utility::bit_pos(VK_SHADER_STAGE_MISS_BIT_NV),//11
		Intersection = Utility::bit_pos(VK_SHADER_STAGE_INTERSECTION_BIT_NV),//12
		Callable = Utility::bit_pos(VK_SHADER_STAGE_CALLABLE_BIT_NV),//13
	};
	constexpr size_t NUM_SHADER_STAGES = 6;
	
	
	class Shader
	{
	public:
		Shader(LogicalDevice& parent, const std::vector<uint32_t>& shaderCode);
		~Shader();
		ShaderStage get_stage();
		void parse_shader(ShaderLayout& layouts, const std::vector<uint32_t>& shaderCode) const;
		VkPipelineShaderStageCreateInfo get_create_info();
	private:
		inline std::tuple<uint32_t, uint32_t, const spirv_cross::SPIRType&> get_values(const spirv_cross::Resource& resource, const spirv_cross::Compiler& comp) const;
		void inline array_info(std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS>& layouts, const spirv_cross::SPIRType& type, uint32_t set, uint32_t binding) const;
		void parse_stage(const std::vector<uint32_t>& shaderCode);
		void create_module(const std::vector<uint32_t>& shaderCode);

		LogicalDevice& m_parent;
		VkShaderModule m_module = VK_NULL_HANDLE;
		ShaderStage m_stage;
		ShaderLayout m_layout;
		
	}; 
	class Program {
	public:
		Program(const std::vector<Shader*>& shaders);
		Shader* get_shader(ShaderStage stage) const;
		void set_pipeline_layout(PipelineLayout* layout);
		PipelineLayout* get_pipeline_layout()const ;
	private:
		void set_shader(Shader* shader);
		PipelineLayout* m_layout;
		std::array<Shader*, NUM_SHADER_STAGES> m_shaders{};
	};
};

#endif //VKSHADER_H