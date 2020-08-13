#ifndef VKSHADER_H
#define VKSHADER_H
#pragma once
#include "VulkanIncludes.h"
#include <vector>
#include "DescriptorSet.h"
#include "Utility.h"

namespace Vulkan {
	struct ShaderLayout {
		std::array<DescriptorSetLayout, MAX_DESCRIPTOR_SETS> descriptors;
		std::bitset<MAX_DESCRIPTOR_SETS> used;
		std::bitset<32> inputs;
		std::bitset<32> outputs;
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
		Vertex = 0,// VK_SHADER_STAGE_VERTEX_BIT,
		TesselationControl = 1, //VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		TesselationEvaluation = 2, //VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		Geometry = 3, //VK_SHADER_STAGE_GEOMETRY_BIT,
		Fragment = 4, //VK_SHADER_STAGE_FRAGMENT_BIT,
		Compute = 5,//VK_SHADER_STAGE_COMPUTE_BIT,
	};
	constexpr size_t NUM_SHADER_STAGES = 6;
	
	class LogicalDevice;
	class PipelineLayout;
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
		std::array<Shader*, NUM_SHADER_STAGES> m_shaders;
	};
};

#endif //VKSHADER_H