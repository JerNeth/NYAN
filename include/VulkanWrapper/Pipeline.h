#ifndef VKPIPELINE_H
#define VKPIPELINE_H
#pragma once
#include <filesystem>

#include <Util>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/PipelineConfig.h"
#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"


namespace vulkan {

	constexpr DynamicGraphicsPipelineState defaultDynamicGraphicsPipelineState{
		.flags {
			DynamicGraphicsPipelineState::DynamicState::ViewportWithCount,
			DynamicGraphicsPipelineState::DynamicState::ScissorWithCount,
			DynamicGraphicsPipelineState::DynamicState::LineWidth,
			DynamicGraphicsPipelineState::DynamicState::DepthBias,
			DynamicGraphicsPipelineState::DynamicState::StencilCompareMask,
			DynamicGraphicsPipelineState::DynamicState::StencilReference,
			DynamicGraphicsPipelineState::DynamicState::StencilWriteMask,
			DynamicGraphicsPipelineState::DynamicState::CullMode,
			DynamicGraphicsPipelineState::DynamicState::FrontFace,
			//DynamicGraphicsPipelineState::DynamicState::PrimitiveTopology, //Clashes with cull mode on NVIDIA even on 516.94 (Supposedly fixed in 473.50)
			DynamicGraphicsPipelineState::DynamicState::DepthTestEnabled,
			DynamicGraphicsPipelineState::DynamicState::DepthWriteEnabled,
			DynamicGraphicsPipelineState::DynamicState::DepthCompareOp,
			DynamicGraphicsPipelineState::DynamicState::DepthBoundsTestEnabled,
			DynamicGraphicsPipelineState::DynamicState::StencilTestEnabled,
			DynamicGraphicsPipelineState::DynamicState::StencilOp,
			DynamicGraphicsPipelineState::DynamicState::DepthBiasEnabled,
			DynamicGraphicsPipelineState::DynamicState::PrimitiveRestartEnabled,
			DynamicGraphicsPipelineState::DynamicState::RasterizerDiscardEnabled,
		},
		.depthWriteEnable {VK_TRUE},
		.depthTestEnable {VK_TRUE},
		.depthBiasEnable {VK_FALSE},
		.depthBoundsTestEnable {VK_FALSE},
		.depthCompareOp {VK_COMPARE_OP_GREATER_OR_EQUAL}, //Use GE because of inverse Z VK_COMPARE_OP_GREATER_OR_EQUAL

		.stencilTestEnable {VK_FALSE},
		.stencilFrontFail {},
		.stencilFrontPass {},
		.stencilFrontDepthFail {},
		.stencilFrontCompareOp {},

		.stencilBackFail {},
		.stencilBackPass {},
		.stencilBackDepthFail {},
		.stencilBackCompareOp {},

		.cullMode {VK_CULL_MODE_BACK_BIT},
		.frontFace {VK_FRONT_FACE_COUNTER_CLOCKWISE},
		//.front_face {VK_FRONT_FACE_CLOCKWISE},
		.primitiveRestartEnable {VK_FALSE},
		.rasterizerDiscardEnable {VK_FALSE},
		.primitiveTopology {VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST},
	};
	constexpr BlendAttachment defaultBlendAttachment{
		.blendEnable {VK_FALSE},
		.srcColorBlend {},
		.colorWriteMask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
	};
	constexpr BlendAttachment alphaBlendAttachment{
		.blendEnable {VK_TRUE},
		.srcColorBlend {VK_BLEND_FACTOR_SRC_ALPHA},
		.dstColorBlend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
		.colorBlendOp {VK_BLEND_OP_ADD},
		.srcAlphaBlend {VK_BLEND_FACTOR_SRC_ALPHA},
		.dstAlphaBlend {VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA},
		.alphaBlendOp {VK_BLEND_OP_ADD},
		.colorWriteMask {VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT}
	};
	constexpr GraphicsPipelineState defaultGraphicsPipelineState{
		.polygonMode {VK_POLYGON_MODE_FILL},
		.rasterizationSamples {VK_SAMPLE_COUNT_1_BIT},
		.logicOpEnable {VK_FALSE},
		.patchControlPoints {0},
		.blendAttachments {defaultBlendAttachment}
	};
	constexpr GraphicsPipelineState alphaBlendedGraphicsPipelineState{
		.polygonMode {VK_POLYGON_MODE_FILL},
		.rasterizationSamples {VK_SAMPLE_COUNT_1_BIT},
		.logicOpEnable {VK_FALSE},
		.patchControlPoints {0},
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
	class PipelineLayout2 : public VulkanObject<VkPipelineLayout> {
	public:
		PipelineLayout2(LogicalDevice& device, const std::vector<VkDescriptorSetLayout>& sets);
		~PipelineLayout2();
		PipelineLayout2(PipelineLayout2&) = delete;
		PipelineLayout2(PipelineLayout2&&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&) = delete;
		PipelineLayout2& operator=(PipelineLayout2&&) = delete;
	private:
	};

	class PipelineCache: public VulkanObject<VkPipelineCache>   {
		//Curtesy of https://zeux.io/2019/07/17/serializing-pipeline-cache/
		struct PipelineCachePrefixHeader {
			static constexpr uint32_t magicNumberValue = 0x68636163u;
			uint32_t magicNumber; // an arbitrary magic header to make sure this is actually our file
			uint32_t dataSize; // equal to *pDataSize returned by vkGetPipelineCacheData
			uint64_t dataHash; // a hash of pipeline cache data, including the header

			uint32_t vendorID; // equal to VkPhysicalDeviceProperties::vendorID
			uint32_t deviceID; // equal to VkPhysicalDeviceProperties::deviceID
			uint32_t driverVersion; // equal to VkPhysicalDeviceProperties::driverVersion
			uint32_t driverABI; // equal to sizeof(void*)

			std::array<uint8_t, VK_UUID_SIZE>  uuid; // equal to VkPhysicalDeviceProperties::pipelineCacheUUID

			friend bool operator==(const PipelineCachePrefixHeader& left, const PipelineCachePrefixHeader& right) {
				return left.magicNumber == right.magicNumber &&
					left.dataSize == right.dataSize &&
					left.dataHash == right.dataHash &&
					left.vendorID == right.vendorID &&
					left.deviceID == right.deviceID &&
					left.driverVersion == right.driverVersion &&
					left.driverABI == right.driverABI &&
					left.uuid == right.uuid;
			}
		};
	public:
		PipelineCache(LogicalDevice& device, std::filesystem::path path);
		PipelineCache(PipelineCache&) = delete;
		PipelineCache(PipelineCache&&) = delete;
		PipelineCache& operator=(PipelineCache&) = delete;
		PipelineCache& operator=(PipelineCache&&) = delete;
		~PipelineCache() noexcept;
	private:
		std::filesystem::path m_path;
	};
	class Pipeline2 : public VulkanObject<VkPipeline> {
	public:
		Pipeline2(LogicalDevice& parent, const GraphicsPipelineConfig& config);
		Pipeline2(LogicalDevice& parent, const ComputePipelineConfig& config);
		Pipeline2(LogicalDevice& parent, const RaytracingPipelineConfig& config);
		VkPipelineLayout get_layout() const noexcept;
		const DynamicGraphicsPipelineState& get_dynamic_state() const noexcept;
	private:
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
			static_assert(sizeof(T) < 128);
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
		void set_scissor_with_count(uint32_t count, VkRect2D* scissor);
		void set_viewport(VkViewport viewport);
		void set_viewport_with_count(uint32_t count, VkViewport* viewport);
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
		void trace_rays(const RTPipeline& pipeline,
			uint32_t width = 1, uint32_t height = 1, uint32_t depth = 1);
		void trace_rays(const RTPipeline& pipeline, VkDeviceAddress address);
	private:
	};

}

#endif