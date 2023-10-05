#pragma once
#ifndef VKPIPELINECONFIG_H
#define VKPIPELINECONFIG_H

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/Utility.h"


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
		VkBool32 blendEnable : 1;
		VkBlendFactor srcColorBlend : BLEND_FACTOR_BITS;
		VkBlendFactor dstColorBlend : BLEND_FACTOR_BITS;
		VkBlendOp colorBlendOp : BLEND_OP_BITS;
		VkBlendFactor srcAlphaBlend : BLEND_FACTOR_BITS;
		VkBlendFactor dstAlphaBlend : BLEND_FACTOR_BITS;
		VkBlendOp alphaBlendOp : BLEND_OP_BITS;
		VkColorComponentFlags colorWriteMask : WRITE_MASK_BITS;
	};

	struct DynamicGraphicsPipelineState {
		enum class DynamicState : uint32_t {
			ViewportWithCount,
			ScissorWithCount,
			LineWidth,
			DepthBias,
			StencilCompareMask,
			StencilReference,
			StencilWriteMask,
			CullMode,
			FrontFace,
			PrimitiveTopology,
			DepthTestEnabled,
			DepthWriteEnabled,
			DepthCompareOp,
			DepthBoundsTestEnabled,
			StencilTestEnabled,
			StencilOp,
			DepthBiasEnabled,
			PrimitiveRestartEnabled,
			RasterizerDiscardEnabled,
			SIZE
		};
		Utility::bitset<static_cast<size_t>(DynamicState::SIZE), DynamicState> flags;
		VkBool32 depthWriteEnable : 1;
		VkBool32 depthTestEnable : 1;
		VkBool32 depthBiasEnable : 1;
		VkBool32 depthBoundsTestEnable : 1;
		VkCompareOp depthCompareOp : COMPARE_OP_BITS;

		VkBool32 stencilTestEnable : 1;
		VkStencilOp stencilFrontFail : STENCIL_OP_BITS;
		VkStencilOp stencilFrontPass : STENCIL_OP_BITS;
		VkStencilOp stencilFrontDepthFail : STENCIL_OP_BITS;
		VkCompareOp stencilFrontCompareOp : COMPARE_OP_BITS;

		VkStencilOp stencilBackFail : STENCIL_OP_BITS;
		VkStencilOp stencilBackPass : STENCIL_OP_BITS;
		VkStencilOp stencilBackDepthFail : STENCIL_OP_BITS;
		VkCompareOp stencilBackCompareOp : COMPARE_OP_BITS;

		VkCullModeFlags cullMode : CULL_MODE_BITS;
		VkFrontFace frontFace : 2;
		VkBool32 primitiveRestartEnable : 1;
		VkBool32 rasterizerDiscardEnable : 1;
		VkPrimitiveTopology primitiveTopology : TOPOLOGY_BITS;
		uint32_t stencilFrontReference : 8;
		uint32_t stencilFrontWriteMask : 8;
		uint32_t stencilFrontCompareMask : 8;
		uint32_t stencilBackReference : 8;
		uint32_t stencilBackWriteMask : 8;
		uint32_t stencilBackCompareMask : 8;
	};

	struct GraphicsPipelineState {
		VkPolygonMode polygonMode : POLYGON_MODE_BITS;

		VkSampleCountFlagBits rasterizationSamples : RASTERIZATION_SAMPLE_BITS;
		VkBool32 alphaToCoverage : 1;
		VkBool32 alphaToOne : 1;
		VkBool32 sampleShading : 1;

		VkBool32 logicOpEnable : 1;
		VkLogicOp logicOp : LOGIC_OP_BITS;

		//All devices supporting tessellation use 32 as max value
		uint32_t patchControlPoints : Utility::bit_width(32);

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
