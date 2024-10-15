module;

#include <array>
#include <cassert>
#include <expected>
#include <utility>
#include <variant>

#include "volk.h"

module NYANVulkan;

using namespace nyan::vulkan;

nyan::vulkan::Pipeline::Pipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle) :
	Object(device, handle),
	r_layout(layout)
{
}

nyan::vulkan::Pipeline::Pipeline(Pipeline&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_layout(other.r_layout)
{
}

Pipeline& nyan::vulkan::Pipeline::operator=(Pipeline&& other) noexcept
{
	if (this != std::addressof(other))
	{
		std::swap(ptr_device, other.ptr_device);
		assert(std::addressof(r_layout) == std::addressof(other.r_layout));
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

nyan::vulkan::Pipeline::~Pipeline() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		ptr_device->vkDestroyPipeline(m_handle);
}



[[nodiscard]] static constexpr VkPipelineRenderingCreateInfo create_rendering_create_info(const GraphicsPipeline::RenderingCreateInfo& renderingCreateInfo, VkFormat* attachments) noexcept {
	for (uint32_t attachment = 0; attachment < renderingCreateInfo.colorAttachmentCount; ++attachment)
		attachments[attachment] = static_cast<VkFormat>(renderingCreateInfo.colorAttachmentFormats[attachment]);
	return VkPipelineRenderingCreateInfo {
		.sType {VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO},
		.pNext {nullptr},
		.viewMask {renderingCreateInfo.viewMask},
		.colorAttachmentCount {renderingCreateInfo.colorAttachmentCount},
		.pColorAttachmentFormats {attachments},
		.depthAttachmentFormat {static_cast<VkFormat>(renderingCreateInfo.depthAttachmentFormat)},
		.stencilAttachmentFormat {static_cast<VkFormat>(renderingCreateInfo.stencilAttachmentFormat)}
	};
}

[[nodiscard]] static constexpr size_t format_bytesize(VkFormat format) {
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
		assert(false);
		return 0;
	}
}

[[nodiscard]] static constexpr VkPipelineVertexInputStateCreateInfo create_vertex_input_state_create_info(const VertexShaderGraphicsPipeline::VertexInput& vertexInput, VkVertexInputBindingDescription* bindingDescriptions, VkVertexInputAttributeDescription* attributeDescriptions) noexcept {
	uint32_t vertexInputCount{ 0 };
	assert(vertexInput.vertexInputCount < GraphicsPipeline::MaxVertexInputs);
	for (uint32_t location = 0; location < vertexInput.vertexInputCount; location++) {
		auto format = vertexInput.vertexInputFormats[location];
		attributeDescriptions[location] = VkVertexInputAttributeDescription{
			.location = static_cast<uint32_t>(location),
			.binding = location,
			.format = static_cast<VkFormat>(format),
			.offset = 0
		};
		bindingDescriptions[location] = VkVertexInputBindingDescription{
				.binding = static_cast<uint32_t>(location),
				.stride = static_cast<uint32_t>(format_byte_size(format)),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
		};
	}

	return VkPipelineVertexInputStateCreateInfo {
			.sType {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO},
			.pNext {nullptr},
			.flags {0}, //reserved for future use
			.vertexBindingDescriptionCount {vertexInputCount},
			.pVertexBindingDescriptions {bindingDescriptions},
			.vertexAttributeDescriptionCount {vertexInputCount},
			.pVertexAttributeDescriptions {attributeDescriptions}
	};
}

[[nodiscard]] static constexpr VkPipelineInputAssemblyStateCreateInfo create_input_assembly_state_create_info(const GraphicsPipeline::PipelineState& pipelineState) noexcept {
	return VkPipelineInputAssemblyStateCreateInfo {
		.sType {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},  //reserved for future use
		.topology {static_cast<VkPrimitiveTopology>(pipelineState.primitiveTopology)},
		.primitiveRestartEnable {static_cast<VkBool32>(pipelineState.primitiveRestartEnable)}
	};
}

[[nodiscard]] static constexpr VkPipelineTessellationStateCreateInfo create_tessellation_state_create_info(const GraphicsPipeline::PipelineState& pipelineState) noexcept {
	return VkPipelineTessellationStateCreateInfo {
			.sType {VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO},
			.pNext {nullptr},
			.flags {0},
			.patchControlPoints {pipelineState.patchControlPoints} //max 32
	};
}

[[nodiscard]] static constexpr VkPipelineViewportStateCreateInfo create_viewport_state_create_info(const GraphicsPipeline::PipelineState& pipelineState, VkViewport& viewport, VkRect2D& scissor) noexcept {
	viewport = VkViewport {
		.x {0.f},
		.y {0.f},
		.width {static_cast<float>(pipelineState.viewport.width)},
		.height {static_cast<float>(pipelineState.viewport.height)},
		.minDepth {0.f},
		.maxDepth {1.f}
	};
	scissor = VkRect2D{
		.offset {
			.x {0},
			.y {0},
		},
		.extent	{
			.width {pipelineState.viewport.width},
			.height {pipelineState.viewport.height}
		}
	};
	return VkPipelineViewportStateCreateInfo
	{
		.sType {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0}, //reserved for future use
		.viewportCount {1},
		.pViewports {&viewport},
		.scissorCount {1},
		.pScissors {&scissor}
	};
}

[[nodiscard]] static constexpr VkPipelineRasterizationStateCreateInfo create_rasterization_state_create_info(const GraphicsPipeline::PipelineState& pipelineState) noexcept {
	return VkPipelineRasterizationStateCreateInfo {
		.sType {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.depthClampEnable {static_cast<VkBool32>(pipelineState.depthClampEnable)},
		.rasterizerDiscardEnable {static_cast<VkBool32>(pipelineState.rasterizerDiscardEnable)},
		.polygonMode {static_cast<VkPolygonMode>(pipelineState.polygonMode)},
		.cullMode {static_cast<VkCullModeFlags>(pipelineState.cullMode)},
		.frontFace {static_cast<VkFrontFace>(pipelineState.frontFace)},
		.depthBiasEnable {static_cast<VkBool32>(pipelineState.depthBiasEnable)},
		.depthBiasConstantFactor {pipelineState.depthBiasConstantFactor},
		.depthBiasClamp {pipelineState.depthBiasClamp},
		.depthBiasSlopeFactor {pipelineState.depthBiasSlopeFactor},
		.lineWidth {pipelineState.lineWidth}
	};
}

[[nodiscard]] static constexpr VkPipelineMultisampleStateCreateInfo create_multisample_state_create_info(const GraphicsPipeline::PipelineState& pipelineState, const uint32_t& sampleMask) noexcept {
	assert(pipelineState.rasterizationSampleCount == 1 && "TODO: test if sample mask works for larger samples");

	return VkPipelineMultisampleStateCreateInfo {
		.sType {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.rasterizationSamples {static_cast<VkSampleCountFlagBits>(pipelineState.rasterizationSampleCount)},
		.sampleShadingEnable {static_cast<VkBool32>(pipelineState.sampleShadingEnable)},
		.minSampleShading {pipelineState.minSampleShading},
		.pSampleMask {&sampleMask}, //Currently not invoking fragment shader when used
		.alphaToCoverageEnable {static_cast<VkBool32>(pipelineState.alphaToCoverageEnable)},
		.alphaToOneEnable {static_cast<VkBool32>(pipelineState.alphaToOneEnable)},
	};
}

[[nodiscard]] static constexpr VkPipelineDepthStencilStateCreateInfo create_depth_stencil_state_create_info(const GraphicsPipeline::PipelineState& pipelineState) noexcept {
	return VkPipelineDepthStencilStateCreateInfo {
		.sType{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO },
		.pNext{ nullptr },
		.flags{ 0 },
		.depthTestEnable{static_cast<VkBool32>(pipelineState.depthTestEnable)},
		.depthWriteEnable{static_cast<VkBool32>(pipelineState.depthWriteEnable)},
		.depthCompareOp{static_cast<VkCompareOp>(pipelineState.depthCompareOp)},
		.depthBoundsTestEnable{static_cast<VkBool32>(pipelineState.depthBoundsTestEnable)},
		.stencilTestEnable{static_cast<VkBool32>(pipelineState.stencilTestEnable)},
		.front{
			.failOp{static_cast<VkStencilOp>(pipelineState.stencilFrontFailOp)},
			.passOp{static_cast<VkStencilOp>(pipelineState.stencilFrontPassOp)},
			.depthFailOp{static_cast<VkStencilOp>(pipelineState.stencilFrontDepthFailOp)},
			.compareOp{static_cast<VkCompareOp>(pipelineState.stencilFrontCompareOp)},
			.compareMask{static_cast<uint32_t>(pipelineState.stencilFrontCompareMask)},
			.writeMask {static_cast<uint32_t>(pipelineState.stencilFrontWriteMask)},
			.reference {static_cast<uint32_t>(pipelineState.stencilFrontReference)}
		},
		.back{
			.failOp{static_cast<VkStencilOp>(pipelineState.stencilBackFailOp)},
			.passOp{static_cast<VkStencilOp>(pipelineState.stencilBackPassOp)},
			.depthFailOp{static_cast<VkStencilOp>(pipelineState.stencilBackDepthFailOp)},
			.compareOp{static_cast<VkCompareOp>(pipelineState.stencilBackCompareOp)},
			.compareMask{static_cast<uint32_t>(pipelineState.stencilBackCompareMask)},
			.writeMask {static_cast<uint32_t>(pipelineState.stencilBackWriteMask)},
			.reference {static_cast<uint32_t>(pipelineState.stencilBackReference)}
		},
		.minDepthBounds{pipelineState.minDepthBounds},
		.maxDepthBounds{pipelineState.maxDepthBounds}
	};
}

[[nodiscard]] static constexpr VkPipelineColorBlendStateCreateInfo create_color_blend_state_create_info(uint32_t attachmentCount, const GraphicsPipeline::PipelineState& pipelineState, VkPipelineColorBlendAttachmentState* attachmentState) noexcept {
	assert(attachmentCount < GraphicsPipeline::MaxAttachments);
	for (uint32_t attachment = 0; attachment < attachmentCount; ++attachment)
		attachmentState[attachment] = VkPipelineColorBlendAttachmentState{
			.blendEnable{static_cast<VkBool32>(pipelineState.blendAttachments[attachment].blendEnable)},
			.srcColorBlendFactor{static_cast<VkBlendFactor>(pipelineState.blendAttachments[attachment].srcColorBlend)},
			.dstColorBlendFactor{static_cast<VkBlendFactor>(pipelineState.blendAttachments[attachment].dstColorBlend)},
			.colorBlendOp{static_cast<VkBlendOp>(pipelineState.blendAttachments[attachment].colorBlendOp)},
			.srcAlphaBlendFactor{static_cast<VkBlendFactor>(pipelineState.blendAttachments[attachment].srcAlphaBlend)},
			.dstAlphaBlendFactor{static_cast<VkBlendFactor>(pipelineState.blendAttachments[attachment].dstAlphaBlend)},
			.alphaBlendOp{static_cast<VkBlendOp>(pipelineState.blendAttachments[attachment].alphaBlendOp)},
			.colorWriteMask{static_cast<VkColorComponentFlags>(pipelineState.blendAttachments[attachment].colorWriteMask)}
		};
	return VkPipelineColorBlendStateCreateInfo{
		.sType {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.logicOpEnable {static_cast<VkBool32>(pipelineState.logicOpEnable)},
		.logicOp {static_cast<VkLogicOp>(pipelineState.logicOp)},
		.attachmentCount {attachmentCount},
		.pAttachments {attachmentState},
		.blendConstants {pipelineState.blendConstants[0], pipelineState.blendConstants[1], 
						pipelineState.blendConstants[2], pipelineState.blendConstants[3]}
	};
}

[[nodiscard]] static constexpr VkPipelineDynamicStateCreateInfo create_dynamic_state_create_info(const GraphicsPipeline::DynamicStates& dynamicStateBitset, VkDynamicState* dynamicStates) noexcept {
	
	uint32_t dynamicStatesCount{ 0 };
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::Viewport)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::Scissor)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SCISSOR;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LineWidth)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LINE_WIDTH;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthBias)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::BlendConstants)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_BLEND_CONSTANTS;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthBounds)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::StencilCompareMask)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::StencilWriteMask)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::StencilReference)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CullMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_CULL_MODE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::FrontFace)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_FRONT_FACE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::PrimitiveTopology))
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportWithCount)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_WITH_COUNT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ScissorWithCount)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SCISSOR_WITH_COUNT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::VertexInputBindingStride)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VERTEX_INPUT_BINDING_STRIDE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthTestEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_TEST_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthWriteEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_WRITE_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthCompareOp)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_COMPARE_OP;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthBoundsTestEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_BOUNDS_TEST_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::StencilTestEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_STENCIL_TEST_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::StencilOp)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_STENCIL_OP;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::RasterizerDiscardEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_RASTERIZER_DISCARD_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthBiasEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_BIAS_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::PrimitiveRestartEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_PRIMITIVE_RESTART_ENABLE;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportWScaling)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DiscardRectangle)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DiscardRectangleEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DiscardRectangleMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DISCARD_RECTANGLE_MODE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::SampleLocations)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportShadingRatePalette)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_SHADING_RATE_PALETTE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportShadingCoarseSampleOrder)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_COARSE_SAMPLE_ORDER_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ExclusiveScissorEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ExclusiveScissor)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_EXCLUSIVE_SCISSOR_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::FragmentShadingRate)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_FRAGMENT_SHADING_RATE_KHR;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::VertexInput)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::PatchControlPoints)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_PATCH_CONTROL_POINTS_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LogicOp)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LOGIC_OP_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ColorWriteEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COLOR_WRITE_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthClampEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_CLAMP_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::PolgyonMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_POLYGON_MODE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::RasterizationSamples)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_RASTERIZATION_SAMPLES_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::SampleMask)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SAMPLE_MASK_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::AlphaToCoverageEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_ALPHA_TO_COVERAGE_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::AlphaToOneEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_ALPHA_TO_ONE_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LogicOpEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LOGIC_OP_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ColorBlendEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COLOR_BLEND_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ColorBlendEquation)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COLOR_BLEND_EQUATION_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ColorWriteMask)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::TesselationDomainOrigin)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_TESSELLATION_DOMAIN_ORIGIN_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::RasterizationStream)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_RASTERIZATION_STREAM_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ConservativeRasterizationMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_CONSERVATIVE_RASTERIZATION_MODE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ExtraPrimitiveOverestimationSize)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_EXTRA_PRIMITIVE_OVERESTIMATION_SIZE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthClipEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_CLIP_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::SampleLocationsEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SAMPLE_LOCATIONS_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ColorBlendAdvanced)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COLOR_BLEND_ADVANCED_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ProvokingVertexMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_PROVOKING_VERTEX_MODE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LineRasterizationMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LINE_RASTERIZATION_MODE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LineStippleEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LINE_STIPPLE_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::DepthClipNegativeOneToOne)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_DEPTH_CLIP_NEGATIVE_ONE_TO_ONE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportWScalingEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_W_SCALING_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ViewportSwizzle)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_VIEWPORT_SWIZZLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageToColorEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageToColorLocation)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_TO_COLOR_LOCATION_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageModulationMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_MODULATION_MODE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageModulationTableEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageModulationTable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_MODULATION_TABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::ShadingRateImageEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_SHADING_RATE_IMAGE_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::RepresentativeFragmentTestEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_REPRESENTATIVE_FRAGMENT_TEST_ENABLE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::CoverageReductionMode)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_COVERAGE_REDUCTION_MODE_NV;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::AttachmentFeedbackLoopEnable)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_ATTACHMENT_FEEDBACK_LOOP_ENABLE_EXT;
	if (dynamicStateBitset.test(GraphicsPipeline::DynamicState::LineStipple)) 
		dynamicStates[dynamicStatesCount++] = VK_DYNAMIC_STATE_LINE_STIPPLE_KHR;

	return VkPipelineDynamicStateCreateInfo{
		.sType{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO},
		.pNext{nullptr},
		.flags {0},
		.dynamicStateCount {dynamicStatesCount},
		.pDynamicStates {dynamicStates}
	};
}

GraphicsPipeline::GraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept :
	Pipeline(device, layout, handle),
	m_pipelineState(pipelineState)
{
	assert(m_handle != VK_NULL_HANDLE);
}

const GraphicsPipeline::PipelineState& GraphicsPipeline::get_pipeline_state() const noexcept
{
	return m_pipelineState;
}

std::expected<MeshShaderGraphicsPipeline, Error> MeshShaderGraphicsPipeline::create(const LogicalDevice& device, const PipelineLayout& layout, const Parameters& params, PipelineCache* pipelineCache) noexcept
{
	assert(false && "TODO");

	constexpr bool useDescriptorBuffers = false;
	
	VkViewport viewport;
	VkRect2D scissor;

	auto viewportStateCreateInfo = create_viewport_state_create_info(params.pipelineState, viewport, scissor);

	std::array<VkFormat, GraphicsPipeline::MaxAttachments> colorAttachmentFormats;

	uint32_t colorAttachmentCount{ 0 };
	VkPipelineRenderingCreateInfo renderingCreateInfo;
	if (std::holds_alternative<RenderingCreateInfo>(params.renderInfo)) {
		const auto& renderingInfo = std::get<RenderingCreateInfo>(params.renderInfo);
		renderingCreateInfo = create_rendering_create_info(renderingInfo, colorAttachmentFormats.data());
		colorAttachmentCount = renderingInfo.colorAttachmentCount;
	}
	else {
		assert(std::holds_alternative<RenderPassInfo>(params.renderInfo));
		const auto& renderPassInfo = std::get<RenderPassInfo>(params.renderInfo);
		colorAttachmentCount = renderPassInfo.renderPass.get_num_color_attachments(renderPassInfo.subpass);
	}

	auto rasterizationStateCreateInfo = create_rasterization_state_create_info(params.pipelineState);

	//optional
	uint32_t sampleMask{ params.pipelineState.sampleMask };
	auto multisampleStateCreateInfo = create_multisample_state_create_info(params.pipelineState, sampleMask);

	auto depthStencilStateCreateInfo = create_depth_stencil_state_create_info(params.pipelineState);

	std::array< VkPipelineColorBlendAttachmentState, GraphicsPipeline::MaxAttachments> blendAttachments;

	auto colorBlendStateCreateInfo = create_color_blend_state_create_info(colorAttachmentCount, params.pipelineState, blendAttachments.data());

	std::array<VkDynamicState, GraphicsPipeline::DynamicStates::size()> dynamicStates;

	if (params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::VertexInputBindingStride) ||
		params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::PrimitiveTopology) ||
		params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::PrimitiveRestartEnable) ||
		params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::PatchControlPoints) ||
		params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::VertexInput)) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	auto dynamicStateCreateInfo = create_dynamic_state_create_info(params.pipelineState.dynamicState, dynamicStates.data());

	uint32_t stageCount{ 0 };
	std::array<VkPipelineShaderStageCreateInfo, 3> shaderStageCreateInfo;

	shaderStageCreateInfo[stageCount++] = params.meshShader.get_shader_stage_create_info();

	if (params.taskShader)
		shaderStageCreateInfo[stageCount++] = params.taskShader->get_shader_stage_create_info();

	shaderStageCreateInfo[stageCount++] = params.fragmentShader.get_shader_stage_create_info();

	VkGraphicsPipelineCreateInfo graphicsCreateInfo{
		.sType {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO},
		.pNext {std::holds_alternative<RenderingCreateInfo>(params.renderInfo) ? &renderingCreateInfo : nullptr},
		.flags { useDescriptorBuffers ? VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT : static_cast<VkPipelineCreateFlags>(0)},
		.stageCount {stageCount},
		.pStages {shaderStageCreateInfo.data()},
		.pVertexInputState {nullptr},
		.pInputAssemblyState {nullptr},
		.pTessellationState {nullptr},
		.pViewportState {&viewportStateCreateInfo},
		.pRasterizationState {&rasterizationStateCreateInfo},
		.pMultisampleState {&multisampleStateCreateInfo},
		.pDepthStencilState {&depthStencilStateCreateInfo},
		.pColorBlendState {&colorBlendStateCreateInfo},
		.pDynamicState {&dynamicStateCreateInfo},
		.layout {layout.get_handle()},
		.renderPass {std::holds_alternative<RenderPassInfo>(params.renderInfo) ? std::get<RenderPassInfo>(params.renderInfo).renderPass.get_handle() : VK_NULL_HANDLE },
		.subpass{ std::holds_alternative<RenderPassInfo>(params.renderInfo) ? std::get<RenderPassInfo>(params.renderInfo).subpass : 0 },
		.basePipelineHandle{VK_NULL_HANDLE},
		.basePipelineIndex {0}
	};

	VkPipeline handle{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateGraphicsPipelines(pipelineCache? pipelineCache->get_handle() : nullptr, 1, &graphicsCreateInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return MeshShaderGraphicsPipeline{ device.get_device(), layout , handle, params.pipelineState };
}

MeshShaderGraphicsPipeline::MeshShaderGraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept :
	GraphicsPipeline(device, layout, handle, pipelineState)
{
}

std::expected<VertexShaderGraphicsPipeline, Error> VertexShaderGraphicsPipeline::create(const LogicalDevice& device, const PipelineLayout& layout, const Parameters& params, PipelineCache* pipelineCache) noexcept
{

	constexpr bool useDescriptorBuffers = false;
	constexpr bool dynamicRendering = true;

	const auto& physicalDevice = device.get_physical_device();
	std::array<VkFormat, GraphicsPipeline::MaxAttachments> colorAttachmentFormats;


	uint32_t colorAttachmentCount{ 0 };
	VkPipelineRenderingCreateInfo renderingCreateInfo;
	if (std::holds_alternative<RenderingCreateInfo>(params.renderInfo)) {
		const auto& renderingInfo = std::get<RenderingCreateInfo>(params.renderInfo);
		renderingCreateInfo = create_rendering_create_info(renderingInfo, colorAttachmentFormats.data());
		colorAttachmentCount = renderingInfo.colorAttachmentCount;
	}
	else {
		assert(std::holds_alternative<RenderPassInfo>(params.renderInfo));
		const auto& renderPassInfo = std::get<RenderPassInfo>(params.renderInfo);
		colorAttachmentCount = renderPassInfo.renderPass.get_num_color_attachments(renderPassInfo.subpass);
	}

	std::array<VkVertexInputBindingDescription, GraphicsPipeline::MaxVertexInputs> bindingDescriptions;
	std::array<VkVertexInputAttributeDescription, GraphicsPipeline::MaxVertexInputs> attributeDescriptions;


	for (uint32_t location = 0; location < params.vertexInput.vertexInputCount; location++) {
		//Probably not necessary since the possible formats are widely supported
		assert(physicalDevice.vertex_format_supported(params.vertexInput.vertexInputFormats[location]));
		if (!physicalDevice.vertex_format_supported(params.vertexInput.vertexInputFormats[location])) [[unlikely]]
			return std::unexpected{ VK_ERROR_FORMAT_NOT_SUPPORTED };
	}

	auto vertexInputStateCreateInfo = create_vertex_input_state_create_info( params.vertexInput, bindingDescriptions.data(), attributeDescriptions.data());

	auto inputAssemblyStateCreateInfo = create_input_assembly_state_create_info(params.pipelineState);

	auto tessellationStateCreateInfo = create_tessellation_state_create_info(params.pipelineState);

	VkViewport viewport;
	VkRect2D scissor;

	auto viewportStateCreateInfo = create_viewport_state_create_info(params.pipelineState, viewport, scissor);

	auto rasterizationStateCreateInfo = create_rasterization_state_create_info(params.pipelineState);

	uint32_t sampleMask{ params.pipelineState.sampleMask };
	auto multisampleStateCreateInfo = create_multisample_state_create_info(params.pipelineState, sampleMask);

	auto depthStencilStateCreateInfo = create_depth_stencil_state_create_info(params.pipelineState);

	std::array< VkPipelineColorBlendAttachmentState, GraphicsPipeline::MaxAttachments> blendAttachments;

	auto colorBlendStateCreateInfo = create_color_blend_state_create_info(colorAttachmentCount, params.pipelineState, blendAttachments.data());

	std::array<VkDynamicState, GraphicsPipeline::DynamicStates::size()> dynamicStates;

	if (params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::ViewportWithCount)) {
		if (viewportStateCreateInfo.viewportCount != 0 || params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::Viewport)) [[unlikely]] {
			assert(false);
			return std::unexpected{ VK_ERROR_UNKNOWN };
		}
	}
	else if (viewportStateCreateInfo.viewportCount == 0) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	if (params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::ScissorWithCount)) {
		if (viewportStateCreateInfo.scissorCount != 0 || params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::Scissor)) [[unlikely]] {
			assert(false);
			return std::unexpected{ VK_ERROR_UNKNOWN };
		}
	}
	else if (viewportStateCreateInfo.scissorCount == 0) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	if(params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::ScissorWithCount) &&
		params.pipelineState.dynamicState.test(GraphicsPipeline::DynamicState::ScissorWithCount) &&
		(viewportStateCreateInfo.scissorCount != viewportStateCreateInfo.viewportCount)) [[unlikely]] {
		assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	auto dynamicStateCreateInfo = create_dynamic_state_create_info(params.pipelineState.dynamicState, dynamicStates.data());

	uint32_t stageCount{ 0 };
	std::array<VkPipelineShaderStageCreateInfo, 5> shaderStageCreateInfo;

	shaderStageCreateInfo[stageCount++] = params.vertexShader.get_shader_stage_create_info();

	if(params.geometryShader)
		shaderStageCreateInfo[stageCount++] = params.geometryShader->get_shader_stage_create_info();

	if (params.tessellationControlShader)
		shaderStageCreateInfo[stageCount++] = params.tessellationControlShader->get_shader_stage_create_info();
	if (params.tessellationEvaluationShader)
		shaderStageCreateInfo[stageCount++] = params.tessellationEvaluationShader->get_shader_stage_create_info();

	if (!(params.tessellationControlShader != nullptr && params.tessellationEvaluationShader != nullptr) &&
		!(params.tessellationControlShader == nullptr && params.tessellationEvaluationShader == nullptr)) [[unlikely]] {
		assert(false && "Must have control AND evalutation shader");
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	shaderStageCreateInfo[stageCount++] = params.fragmentShader.get_shader_stage_create_info();


	VkGraphicsPipelineCreateInfo graphicsCreateInfo{
		.sType {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO},
		.pNext{ std::holds_alternative<RenderingCreateInfo>(params.renderInfo) ? &renderingCreateInfo : nullptr },
		.flags { useDescriptorBuffers ? VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT : static_cast<VkPipelineCreateFlags>(0)},
		.stageCount {stageCount},
		.pStages {shaderStageCreateInfo.data()},
		.pVertexInputState {&vertexInputStateCreateInfo},
		.pInputAssemblyState {&inputAssemblyStateCreateInfo},
		.pTessellationState {params.tessellationControlShader? &tessellationStateCreateInfo : nullptr},
		.pViewportState {&viewportStateCreateInfo},
		.pRasterizationState {&rasterizationStateCreateInfo},
		.pMultisampleState {&multisampleStateCreateInfo},
		.pDepthStencilState {&depthStencilStateCreateInfo},
		.pColorBlendState {&colorBlendStateCreateInfo},
		.pDynamicState {&dynamicStateCreateInfo},
		.layout {layout.get_handle()},
		.renderPass {std::holds_alternative<RenderPassInfo>(params.renderInfo) ? std::get<RenderPassInfo>(params.renderInfo).renderPass.get_handle() : VK_NULL_HANDLE },
		.subpass{ std::holds_alternative<RenderPassInfo>(params.renderInfo) ? std::get<RenderPassInfo>(params.renderInfo).subpass : 0 },
		.basePipelineHandle{VK_NULL_HANDLE},
		.basePipelineIndex {-1}
	};

	VkPipeline handle{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateGraphicsPipelines(pipelineCache ? pipelineCache->get_handle() : nullptr, 1, &graphicsCreateInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return VertexShaderGraphicsPipeline{ device.get_device(), layout , handle, params.pipelineState };
}

VertexShaderGraphicsPipeline::VertexShaderGraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept :
	GraphicsPipeline(device, layout, handle, pipelineState)
{
}

std::expected<ComputePipeline, Error> ComputePipeline::create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, const ShaderInstance& computeShader, PipelineCache* pipelineCache) noexcept
{
	VkPipelineLayout pipelineLayout { layout.get_handle()};
	assert(pipelineLayout != VK_NULL_HANDLE);
	//assert(config.shaderInstance != invalidShaderId);
	VkComputePipelineCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.pNext = nullptr,
		.flags = 0, // |VK_PIPELINE_CREATE_DISPATCH_BASE_BIT 
		.stage = computeShader.get_shader_stage_create_info(), 
		.layout = pipelineLayout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = -1
	};
	//Validate VUID-VkComputePipelineCreateInfo-flags-xxxxx
	assert(!(createInfo.flags & (VK_PIPELINE_CREATE_LIBRARY_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_ANY_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_CLOSEST_HIT_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_MISS_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_NO_NULL_INTERSECTION_SHADERS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_TRIANGLES_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SKIP_AABBS_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_SHADER_GROUP_HANDLE_CAPTURE_REPLAY_BIT_KHR |
		VK_PIPELINE_CREATE_RAY_TRACING_ALLOW_MOTION_BIT_NV |
		VK_PIPELINE_CREATE_INDIRECT_BINDABLE_BIT_NV)));
	assert(createInfo.stage.stage == VK_SHADER_STAGE_COMPUTE_BIT);

	VkPipeline handle{ VK_NULL_HANDLE };
	if (auto result = device.vkCreateComputePipelines(pipelineCache ? pipelineCache->get_handle() : nullptr, 1, &createInfo, &handle);
		result != VK_SUCCESS && result != VK_PIPELINE_COMPILE_REQUIRED_EXT) [[unlikely]] {
		return std::unexpected{result};
	}
	return ComputePipeline{device, layout, handle};
}

ComputePipeline::ComputePipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle) noexcept :
	Pipeline(device, layout, handle)
{
	assert(m_handle != VK_NULL_HANDLE);
}

std::expected<RayTracingPipeline, Error> RayTracingPipeline::create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, PipelineCache* pipelineCache) noexcept
{
	assert(false && "TODO");
	VkPipeline handle{ VK_NULL_HANDLE };
	return RayTracingPipeline{device, layout, handle };
}

RayTracingPipeline::RayTracingPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle) noexcept :
	Pipeline(device, layout, handle)
{
	assert(m_handle != VK_NULL_HANDLE);
}
