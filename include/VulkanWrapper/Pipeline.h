#ifndef VKPIPELINE_H
#define VKPIPELINE_H
#pragma once
#include "VulkanIncludes.h"
#include <vector>
#include "Utility.h"
#include "Shader.h"
#include "Renderpass.h"

namespace Vulkan {
	class Renderpass;
	class LogicalDevice;
	constexpr unsigned BLEND_FACTOR_BITS = Utility::bit_width(VK_BLEND_FACTOR_END_RANGE);
	constexpr unsigned BLEND_OP_BITS = Utility::bit_width(VK_BLEND_OP_END_RANGE);
	constexpr unsigned WRITE_MASK_BITS = Utility::bit_width(VK_COLOR_COMPONENT_A_BIT);
	constexpr unsigned CULL_MODE_BITS = Utility::bit_width(VK_CULL_MODE_FRONT_AND_BACK);
	constexpr unsigned FRONT_FACE_BITS = Utility::bit_width(VK_FRONT_FACE_END_RANGE);
	constexpr unsigned COMPARE_OP_BITS = Utility::bit_width(VK_COMPARE_OP_END_RANGE);
	constexpr unsigned STENCIL_OP_BITS = Utility::bit_width(VK_STENCIL_OP_END_RANGE);
	class Shader;
	struct PipelineState {
		unsigned depth_write : 1;
		unsigned depth_test : 1;
		unsigned blend_enabled : 1;

		VkCullModeFlags cull_mode : CULL_MODE_BITS;
		VkFrontFace front_face : FRONT_FACE_BITS;
		unsigned depth_bias_enable : 1;

		unsigned depth_compare : COMPARE_OP_BITS;

		unsigned stencil_test : 1;
		VkStencilOp stencil_front_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_front_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_front_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_front_compare_op : COMPARE_OP_BITS;
		VkStencilOp stencil_back_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_back_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_back_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_back_compare_op : COMPARE_OP_BITS;

		unsigned alpha_to_coverage : 1;
		unsigned alpha_to_one : 1;
		unsigned sample_shading : 1;

		VkBlendFactor src_color_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_color_blend : BLEND_FACTOR_BITS;
		VkBlendOp color_blend_op : BLEND_OP_BITS;
		VkBlendFactor src_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendOp alpha_blend_op : BLEND_OP_BITS;
		VkColorComponentFlags color_write_mask : WRITE_MASK_BITS * MAX_ATTACHMENTS;

		unsigned primitive_restart : 1;
		VkPrimitiveTopology topology : Utility::bit_width(VK_PRIMITIVE_TOPOLOGY_END_RANGE);

		VkPolygonMode wireframe : 1;
		unsigned subgroup_control_size : 1;
		unsigned subgroup_full_group : 1;
		unsigned subgroup_min_size_log2 : 3;
		unsigned subgroup_max_size_log2 : 3;
		unsigned conservative_raster : 1;
		unsigned padding : 13;
	};
	constexpr size_t size = sizeof(PipelineState);
	static_assert(size == 16);
	struct PipelineCompile {
		PipelineState state;
		PipelineLayout* layout;
		Program* program;
		Renderpass* comatibleRenderPass;

		uint32_t subpassIndex;
	};
	class PipelineLayout {
	public:
		PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout);
		~PipelineLayout();
		VkPipelineLayout get_layout() const;
		const ShaderLayout& get_resourceLayout();
	private:
		LogicalDevice& r_parent;
		ShaderLayout m_resourceLayout;
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
	};
	class Pipeline {
	public:
		Pipeline(LogicalDevice& parent, const PipelineCompile& compiled);
		~Pipeline();
	private:
		template<typename VertexType, size_t numShaders>
		void create_graphics_pipeline(std::array<Shader*, numShaders> shaders);


		LogicalDevice& r_parent;
		float m_width;
		float m_height;

		VkPipeline m_pipeline = VK_NULL_HANDLE;
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
	};
}

#endif