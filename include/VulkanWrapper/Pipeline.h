#ifndef VKPIPELINE_H
#define VKPIPELINE_H
#pragma once
#include "VulkanIncludes.h"
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
		unsigned blend_enable : 1;

		unsigned cull_mode : CULL_MODE_BITS;
		unsigned front_face : FRONT_FACE_BITS;
		unsigned depth_bias_enable : 1;

		unsigned depth_compare : COMPARE_OP_BITS;

		unsigned stencil_test : 1;
		unsigned stencil_front_fail : STENCIL_OP_BITS;
		unsigned stencil_front_pass : STENCIL_OP_BITS;
		unsigned stencil_front_depth_fail : STENCIL_OP_BITS;
		unsigned stencil_front_compare_op : COMPARE_OP_BITS;
		unsigned stencil_back_fail : STENCIL_OP_BITS;
		unsigned stencil_back_pass : STENCIL_OP_BITS;
		unsigned stencil_back_depth_fail : STENCIL_OP_BITS;
		unsigned stencil_back_compare_op : COMPARE_OP_BITS;

		unsigned alpha_to_coverage : 1;
		unsigned alpha_to_one : 1;
		unsigned sample_shading : 1;

		unsigned src_color_blend : BLEND_FACTOR_BITS;
		unsigned dst_color_blend : BLEND_FACTOR_BITS;
		unsigned color_blend_op : BLEND_OP_BITS;
		unsigned src_alpha_blend : BLEND_FACTOR_BITS;
		unsigned dst_alpha_blend : BLEND_FACTOR_BITS;
		unsigned alpha_blend_op : BLEND_OP_BITS;
		unsigned color_write_mask : WRITE_MASK_BITS * MAX_ATTACHMENTS;

		unsigned primitive_restart : 1;
		unsigned topology : Utility::bit_width(VK_PRIMITIVE_TOPOLOGY_END_RANGE);

		unsigned wireframe : 1;
		unsigned subgroup_control_size : 1;
		unsigned subgroup_full_group : 1;
		unsigned subgroup_min_size_log2 : 3;
		unsigned subgroup_max_size_log2 : 3;
		unsigned conservative_raster : 1;
		unsigned padding : 17{};
		friend bool operator==(const PipelineState& left, const PipelineState& right) {
			return std::memcmp(&left, &right, sizeof(PipelineState))== 0;
		}
	};
	static_assert(sizeof(PipelineState) == 16);
	constexpr PipelineState defaultPipelineState {
		.depth_write = VK_TRUE,
		.depth_test = VK_TRUE,
		.blend_enable = VK_FALSE,
		.cull_mode = VK_CULL_MODE_FRONT_BIT,
		.front_face = VK_FRONT_FACE_CLOCKWISE,
		.depth_bias_enable = VK_FALSE,
		.depth_compare = VK_COMPARE_OP_LESS,
		.stencil_test = VK_FALSE,
		.alpha_to_coverage = VK_FALSE,
		.alpha_to_one = VK_FALSE,
		.sample_shading = VK_TRUE,
		.src_color_blend = VK_BLEND_FACTOR_ONE,
		.dst_color_blend = VK_BLEND_FACTOR_ZERO,
		.color_blend_op = VK_BLEND_OP_ADD,
		.src_alpha_blend = VK_BLEND_FACTOR_ONE,
		.dst_alpha_blend = VK_BLEND_FACTOR_ZERO,
		.alpha_blend_op = VK_BLEND_OP_ADD,
		.color_write_mask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
		.primitive_restart = VK_FALSE,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		.wireframe = VK_POLYGON_MODE_FILL,
		.subgroup_control_size = VK_FALSE,
		.subgroup_full_group = VK_FALSE,
		.subgroup_min_size_log2 = 0,
		.subgroup_max_size_log2 = 0,
		.conservative_raster = 0,
		.padding {}
	};
	struct PipelineCompile {
		PipelineState state;
		Program* program;
		Renderpass* compatibleRenderPass;

		uint32_t subpassIndex;
	};
	class PipelineLayout {
	public:
		PipelineLayout(LogicalDevice& parent, const ShaderLayout& layout);
		~PipelineLayout();
		const VkPipelineLayout& get_layout() const;
		const ShaderLayout& get_resourceLayout() const;
		const DescriptorSetAllocator* get_allocator(size_t set) const;
		const VkDescriptorUpdateTemplate& get_update_template(size_t set) const;
		
	private:
		void create_update_template();
		LogicalDevice& r_device;
		ShaderLayout m_resourceLayout;
		std::array<DescriptorSetAllocator*, MAX_DESCRIPTOR_SETS> m_descriptors{};
		std::array<VkDescriptorUpdateTemplate, MAX_DESCRIPTOR_SETS> m_updateTemplate{};
		VkPipelineLayout m_layout = VK_NULL_HANDLE;
	};
	class Pipeline {
	public:
		Pipeline(LogicalDevice& parent, const PipelineCompile& compiled);
		~Pipeline() noexcept;
		Pipeline(Pipeline& other) = delete;
		Pipeline(Pipeline&& other);
		const Pipeline& operator=(Pipeline& other) = delete;
		const Pipeline& operator=(Pipeline&&) = delete;
		VkPipeline get_pipeline() const noexcept;
		static Pipeline request_pipeline(LogicalDevice& parent, Program* program, Renderpass* compatibleRenderPass, uint32_t subpassIndex);
		static void reset_static_pipeline();
		static void set_depth_write(bool depthWrite);
		static void set_depth_test(bool depthTest);
		static void set_blend_enabled(bool blendEnabled);
		static void set_cull_mode(VkCullModeFlags cullMode);
		static void set_front_face(VkFrontFace frontFace);
		static void set_depth_bias_enabled(bool depthBiasEnabled);
		static void set_stencil_test_enabled(bool stencilTestEnabled);
		static void set_stencil_front_fail(VkStencilOp frontFail);
		static void set_stencil_front_pass(VkStencilOp frontPass);
		static void set_stencil_front_depth_fail(VkStencilOp frontDepthFail);
		static void set_stencil_front_compare_op(VkCompareOp frontCompareOp);
		static void set_stencil_back_fail(VkStencilOp backFail);
		static void set_stencil_back_pass(VkStencilOp backPass);
		static void set_stencil_back_depth_fail(VkStencilOp backDepthFail);
		static void set_stencil_back_compare_op(VkCompareOp backCompareOp);
		static void set_alpha_to_coverage(bool alphaToCoverage);
		static void set_alpha_to_one(bool alphaToOne);
		static void set_sample_shading(bool sampleShading);
		static void set_src_color_blend(VkBlendFactor srcColorBlend);
		static void set_dst_color_blend(VkBlendFactor dstColorBlend);
		static void set_color_blend_op(VkBlendOp colorBlendOp);
		static void set_src_alpha_blend(VkBlendFactor srcAlphaBlend);
		static void set_dst_alpha_blend(VkBlendFactor dstAlphaBlend);
		static void set_alpha_blend_op(VkBlendOp alphaBlendOp);
		static void set_color_write_mask(VkColorComponentFlags writeMask, uint32_t colorAttachment);
		static void set_primitive_restart(bool primitiveRestart);
		static void set_topology(VkPrimitiveTopology primitiveTopology);
		static void set_wireframe(VkPolygonMode wireframe);
		static void set_subgroup_control_size(bool controlSize);
		static void set_subgroup_full_group(bool fullGroup);
		static void set_subgroup_min_size_log2(unsigned subgroupMinSize);
		static void set_subgroup_max_size_log2(unsigned subgroupMaxSize);
		static void set_conservative_raster(bool conservativeRaster);

	private:


		LogicalDevice& r_device;

		VkPipeline m_pipeline = VK_NULL_HANDLE;
		static PipelineState s_pipelineState;
		
	};
}

#endif