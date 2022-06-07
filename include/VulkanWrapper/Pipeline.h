#ifndef VKPIPELINE_H
#define VKPIPELINE_H
#pragma once
#include "VulkanIncludes.h"
#include <Util>
#include "VulkanForwards.h"

namespace vulkan {
	constexpr unsigned BLEND_FACTOR_BITS = Utility::bit_width(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA) + 1 ;
	constexpr unsigned BLEND_OP_BITS = Utility::bit_width(VK_BLEND_OP_MAX) + 1 ;
	constexpr unsigned WRITE_MASK_BITS = Utility::bit_width(VK_COLOR_COMPONENT_A_BIT) + 1 ;
	constexpr unsigned CULL_MODE_BITS = Utility::bit_width(VK_CULL_MODE_FRONT_AND_BACK) + 1 ;
	constexpr unsigned FRONT_FACE_BITS = Utility::bit_width(VK_FRONT_FACE_CLOCKWISE) + 1 ;
	constexpr unsigned COMPARE_OP_BITS = Utility::bit_width(VK_COMPARE_OP_ALWAYS) + 1 ;
	constexpr unsigned STENCIL_OP_BITS = Utility::bit_width(VK_STENCIL_OP_DECREMENT_AND_WRAP) + 1 ;
	constexpr unsigned TOPOLOGY_BITS = Utility::bit_width(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) + 1 ;
	constexpr unsigned LOGIC_OP_BITS = Utility::bit_width(VK_LOGIC_OP_SET) + 1 ;
	constexpr unsigned POLYGON_MODE_BITS = Utility::bit_width(VK_POLYGON_MODE_POINT) + 1 ;
	constexpr unsigned RASTERIZATION_SAMPLE_BITS = Utility::bit_width(VK_SAMPLE_COUNT_16_BIT) + 1 ;

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
	constexpr DynamicGraphicsPipelineState defaultDynamicGraphicsPipelineState{
		.depth_write_enable {VK_TRUE},
		.depth_test_enable {VK_TRUE},
		.depth_bias_enable {VK_FALSE},
		.depth_bounds_test_enable {VK_FALSE},
		.depth_compare_op {VK_COMPARE_OP_GREATER_OR_EQUAL}, //Use GE because of inverse Z VK_COMPARE_OP_GREATER_OR_EQUAL

		.stencil_test_enable {VK_FALSE},
		.stencil_front_fail {},
		.stencil_front_pass {},
		.stencil_front_depth_fail {},
		.stencil_front_compare_op {},

		.stencil_back_fail {},
		.stencil_back_pass {},
		.stencil_back_depth_fail {},
		.stencil_back_compare_op {},

		.cull_mode {VK_CULL_MODE_BACK_BIT},
		.front_face {VK_FRONT_FACE_COUNTER_CLOCKWISE},
		.primitive_restart_enable {VK_FALSE},
		.rasterizer_discard_enable {VK_FALSE},
		.primitive_topology {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
	};
	constexpr BlendAttachment defaultBlendAttachment{
		.blend_enable {VK_FALSE},
		.src_color_blend {},
		.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
	};
	constexpr BlendAttachment alphaBlendAttachment{
		.blend_enable {VK_TRUE},
		.src_color_blend {VK_BLEND_FACTOR_SRC_ALPHA},
		.dst_color_blend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
		.color_blend_op {VK_BLEND_OP_ADD},
		.src_alpha_blend {VK_BLEND_FACTOR_SRC_ALPHA},
		.dst_alpha_blend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
		.alpha_blend_op {VK_BLEND_OP_ADD},
		.color_write_mask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
	};
	constexpr GraphicsPipelineState defaultGraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments {defaultBlendAttachment}
	};
	constexpr GraphicsPipelineState alphaBlendedGraphicsPipelineState{
		.polygon_mode {VK_POLYGON_MODE_FILL},
		.rasterization_samples {VK_SAMPLE_COUNT_1_BIT},
		.logic_op_enable {VK_FALSE},
		.patch_control_points {0},
		.blendAttachments { alphaBlendAttachment }
	};
	// Blend Logic Pseudo Code
	//if (blendEnable) {
	//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
	//	finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
	//}
	//else {
	//	finalColor = newColor;
	//}

	//finalColor = finalColor & colorWriteMask;
	struct RenderingCreateInfo {
		uint32_t colorAttachmentCount;
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
		ShaderId shaderInstance;
		VkPipelineLayout pipelineLayout;
	};
	struct Group {
		ShaderId generalShader { invalidShaderId };
		ShaderId closestHitShader{ invalidShaderId };
		ShaderId anyHitShader { invalidShaderId };
		ShaderId intersectionShader { invalidShaderId };
	};
	struct RaytracingPipelineConfig {
		//std::vector<ShaderId> shaders;
		//std::vector<Group> groups;
		std::vector<Group> rgenGroups{};
		std::vector<Group> hitGroups{};
		std::vector<Group> missGroups{};
		std::vector<Group> callableGroups {};
		uint32_t recursionDepth{0};
		VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
	};
	class PipelineLayout2 {
	public:
		PipelineLayout2(LogicalDevice& device, const std::vector<VkDescriptorSetLayout>& sets);
		~PipelineLayout2();
		PipelineLayout2(PipelineLayout2&) = delete;
		PipelineLayout2(PipelineLayout2&&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&&) = delete;

		operator VkPipelineLayout() const;
		VkPipelineLayout get_layout() const noexcept;

	private:
		LogicalDevice& r_device;
		VkPipelineLayout m_layout{ VK_NULL_HANDLE };
	};

	class PipelineCache {
	public:
		PipelineCache(LogicalDevice& device, const std::string& path);
		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&&) = delete;
		PipelineCache& operator=(PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) = delete;
		~PipelineCache() noexcept;
		VkPipelineCache get_handle() const noexcept;
	private:
		LogicalDevice& r_parent;
		std::string m_path;
		VkPipelineCache m_handle;
	};
	class Pipeline2 {
	public:
		Pipeline2(LogicalDevice& parent, const GraphicsPipelineConfig& config);
		Pipeline2(LogicalDevice& parent, const ComputePipelineConfig& config);
		Pipeline2(LogicalDevice& parent, const RaytracingPipelineConfig& config);
		VkPipeline get_pipeline() const noexcept;
		operator VkPipeline() const noexcept;
		VkPipelineLayout get_layout() const noexcept;
		const DynamicGraphicsPipelineState& get_dynamic_state() const noexcept;
	private:
		VkPipeline m_pipeline { VK_NULL_HANDLE };
		VkPipelineLayout m_layout { VK_NULL_HANDLE };
		VkPipelineBindPoint m_type { VK_PIPELINE_BIND_POINT_GRAPHICS };
		DynamicGraphicsPipelineState m_initialDynamicState {};
	};

	class PipelineStorage2 {
	public:
		PipelineStorage2(LogicalDevice& device);
		~PipelineStorage2();
		Pipeline2* get_pipeline(PipelineId pipelineId);
		const Pipeline2* get_pipeline(PipelineId pipelineId) const;
		PipelineId add_pipeline(const ComputePipelineConfig& config);
		PipelineId add_pipeline(const GraphicsPipelineConfig& config);
		PipelineId add_pipeline(const RaytracingPipelineConfig& config);
	private:
		LogicalDevice& r_device;
		Utility::LinkedBucketList<Pipeline2> m_pipelines;
	};

	class PipelineBind {
	public:
		PipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void bind_descriptor_sets(uint32_t firstSet, const std::vector<VkDescriptorSet>& descriptorSets,
			const std::vector<uint32_t>& dynamicOffsets = {});
		//void bind_descriptor_set(DescriptorSet)
		template<typename T>
		void push_constants(const T& t) {
			vkCmdPushConstants(m_cmd, m_layout, VK_SHADER_STAGE_ALL, 0, sizeof(T), &t);
		}
	protected:
		VkCommandBuffer m_cmd;
		VkPipelineLayout m_layout;
		VkPipelineBindPoint m_bindPoint;
	};

	class GraphicsPipelineBind : public PipelineBind {
	public:
		GraphicsPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void set_depth_bias_enabled(bool enabled);
		void set_depth_write_enabled(bool enabled);
		void set_depth_test_enabled(bool enabled);
		void set_depth_bounds_test_enabled(bool enabled);
		void set_depth_compare_op(VkCompareOp compareOp);
		void set_stencil_front_reference(uint32_t reference);
		void set_stencil_back_reference(uint32_t reference);
		void set_stencil_front_write_mask(uint32_t mask);
		void set_stencil_back_write_mask(uint32_t mask);
		void set_stencil_front_compare_mask(uint32_t mask);
		void set_stencil_back_compare_mask(uint32_t mask);
		void set_stencil_test_enabled(bool enabled);
		void set_stencil_front_ops(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
		void set_stencil_back_ops(VkStencilOp failOp, VkStencilOp passOp, VkStencilOp depthFailOp, VkCompareOp compareOp);
		void set_cull_mode(VkCullModeFlags cullMode);
		void set_front_face(VkFrontFace frontFace);
		void set_primitive_restart_enable(bool enabled);
		void set_rasterizer_discard_enable(bool enabled);
		void set_primitive_topology(VkPrimitiveTopology topology);
		void set_scissor(VkRect2D scissor);
		void set_viewport(VkViewport viewport);
		void bind_vertex_buffers(uint32_t firstBinding, uint32_t bindingCount, const VkBuffer* buffers, const VkDeviceSize* offsets);
		void bind_index_buffer(VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, uint32_t vertexOffset = 0, uint32_t firstInstance = 0);
		//void draw();
		//
		//void set_something_dynamic();
	private:

	};
	class ComputePipelineBind : public PipelineBind {
	public:
		ComputePipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1);
		//void dispatch
	private:

	};
	class RaytracingPipelineBind : public PipelineBind {
	public:
		RaytracingPipelineBind(VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint);
		void trace_rays(const VkStridedDeviceAddressRegionKHR* raygenSBT, const VkStridedDeviceAddressRegionKHR* missSBT,
			const VkStridedDeviceAddressRegionKHR* hitSBT, const VkStridedDeviceAddressRegionKHR* callableSBT,
			uint32_t width = 1, uint32_t height = 1, uint32_t depth = 1);
	private:
	};

}

#endif