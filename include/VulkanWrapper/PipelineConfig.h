#pragma once
#ifndef VKPIPELINECONFIG_H
#define VKPIPELINECONFIG_H

namespace vulkan {
	constexpr unsigned BLEND_FACTOR_BITS = Utility::bit_width(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA) + 1;
	constexpr unsigned BLEND_OP_BITS = Utility::bit_width(VK_BLEND_OP_MAX) + 1;
	constexpr unsigned WRITE_MASK_BITS = Utility::bit_width(VK_COLOR_COMPONENT_A_BIT) + 1;
	constexpr unsigned CULL_MODE_BITS = Utility::bit_width(VK_CULL_MODE_FRONT_AND_BACK) + 1;
	constexpr unsigned FRONT_FACE_BITS = Utility::bit_width(VK_FRONT_FACE_CLOCKWISE) + 1;
	constexpr unsigned COMPARE_OP_BITS = Utility::bit_width(VK_COMPARE_OP_ALWAYS) + 1;
	constexpr unsigned STENCIL_OP_BITS = Utility::bit_width(VK_STENCIL_OP_DECREMENT_AND_WRAP) + 1;
	constexpr unsigned TOPOLOGY_BITS = Utility::bit_width(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) + 1;
	constexpr unsigned LOGIC_OP_BITS = Utility::bit_width(VK_LOGIC_OP_SET) + 1;
	constexpr unsigned POLYGON_MODE_BITS = Utility::bit_width(VK_POLYGON_MODE_POINT) + 1;
	constexpr unsigned RASTERIZATION_SAMPLE_BITS = Utility::bit_width(VK_SAMPLE_COUNT_16_BIT) + 1;

	struct BlendAttachment {
		VkBool32 blend_enable : 1;
		VkBlendFactor src_color_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_color_blend : BLEND_FACTOR_BITS;
		VkBlendOp color_blend_op : BLEND_OP_BITS;
		VkBlendFactor src_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendFactor dst_alpha_blend : BLEND_FACTOR_BITS;
		VkBlendOp alpha_blend_op : BLEND_OP_BITS;
		VkColorComponentFlags color_write_mask : WRITE_MASK_BITS;
	};

	struct DynamicGraphicsPipelineState {
		VkBool32 depth_write_enable : 1;
		VkBool32 depth_test_enable : 1;
		VkBool32 depth_bias_enable : 1;
		VkBool32 depth_bounds_test_enable : 1;
		VkCompareOp depth_compare_op : COMPARE_OP_BITS;

		VkBool32 stencil_test_enable : 1;
		VkStencilOp stencil_front_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_front_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_front_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_front_compare_op : COMPARE_OP_BITS;

		VkStencilOp stencil_back_fail : STENCIL_OP_BITS;
		VkStencilOp stencil_back_pass : STENCIL_OP_BITS;
		VkStencilOp stencil_back_depth_fail : STENCIL_OP_BITS;
		VkCompareOp stencil_back_compare_op : COMPARE_OP_BITS;

		VkCullModeFlags cull_mode : CULL_MODE_BITS;
		VkFrontFace front_face : 2;
		VkBool32 primitive_restart_enable : 1;
		VkBool32 rasterizer_discard_enable : 1;
		VkPrimitiveTopology primitive_topology : TOPOLOGY_BITS;
		uint32_t stencil_front_reference : 8;
		uint32_t stencil_front_write_mask : 8;
		uint32_t stencil_front_compare_mask : 8;
		uint32_t stencil_back_reference : 8;
		uint32_t stencil_back_write_mask : 8;
		uint32_t stencil_back_compare_mask : 8;
	};

	struct GraphicsPipelineState {
		VkPolygonMode polygon_mode : POLYGON_MODE_BITS;

		VkSampleCountFlagBits rasterization_samples : RASTERIZATION_SAMPLE_BITS;
		VkBool32 alpha_to_coverage : 1;
		VkBool32 alpha_to_one : 1;
		VkBool32 sample_shading : 1;

		VkBool32 logic_op_enable : 1;
		VkLogicOp logic_op : LOGIC_OP_BITS;

		//All devices supporting tessellation use 32 as max value
		uint32_t patch_control_points : Utility::bit_width(32);

		std::array<BlendAttachment, MAX_ATTACHMENTS> blendAttachments{};

		friend bool operator==(const GraphicsPipelineState& left, const GraphicsPipelineState& right) {
			return memcmp(&left, &right, sizeof(GraphicsPipelineState)) == 0;
		}
	};
	struct RenderingCreateInfo {
		uint32_t colorAttachmentCount{ 0 };
		uint32_t viewMask = 0;
		std::array<VkFormat, MAX_ATTACHMENTS> colorAttachmentFormats{ VK_FORMAT_UNDEFINED };
		VkFormat depthAttachmentFormat = VK_FORMAT_UNDEFINED;
		VkFormat stencilAttachmentFormat = VK_FORMAT_UNDEFINED;
	};
	struct GraphicsPipelineConfig {
		DynamicGraphicsPipelineState dynamicState;
		GraphicsPipelineState state;
		RenderingCreateInfo renderingCreateInfo;
		uint16_t vertexInputCount;
		uint16_t shaderCount;
		//Relevant vertex formats only go up to ~128
		std::array<uint8_t, MAX_VERTEX_INPUTS> vertexInputFormats;
		//Five 
		std::array<ShaderId, 5> shaderInstances;
		VkPipelineLayout pipelineLayout;
	};
	struct ComputePipelineConfig {
		ShaderId shaderInstance{ invalidShaderId };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	};
	struct Group {
		ShaderId generalShader{ invalidShaderId };
		ShaderId closestHitShader{ invalidShaderId };
		ShaderId anyHitShader{ invalidShaderId };
		ShaderId intersectionShader{ invalidShaderId };
	};
	struct RaytracingPipelineConfig {
		//std::vector<ShaderId> shaders;
		//std::vector<Group> groups;
		std::vector<Group> rgenGroups{};
		std::vector<Group> hitGroups{};
		std::vector<Group> missGroups{};
		std::vector<Group> callableGroups{};
		uint32_t recursionDepth{ 0 };
		VkPipelineLayout pipelineLayout{ VK_NULL_HANDLE };
	};
}

#endif // 
