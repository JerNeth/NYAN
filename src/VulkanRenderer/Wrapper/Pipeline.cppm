module;

//#include <bit>
//#include <expected>
//#include <limits>
//#include <variant>

#include "magic_enum/magic_enum.hpp"
#include "magic_enum/magic_enum_containers.hpp"

#include "volk.h"

export module NYANVulkan:Pipeline;
import std;

import NYANData;

import :Error;
import :Formats;
import :LogicalDeviceWrapper;
import :Object;
import :PipelineCache;
import :PipelineLayout;
import :Shader;



export namespace nyan::vulkan
{
	class RenderPass;
	class Pipeline : public Object<VkPipeline>
	{
	public:
		const PipelineLayout& get_layout() const noexcept {
			return r_layout;
		}
	protected:
		Pipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle);
		Pipeline(Pipeline&) = delete;
		Pipeline(Pipeline&&) noexcept;

		Pipeline& operator=(Pipeline&) = delete;
		Pipeline& operator=(Pipeline&&) noexcept;

		~Pipeline() noexcept;
		const PipelineLayout& r_layout;
	};

	class GraphicsPipeline : public Pipeline
	{
	protected:
		static constexpr uint32_t BlendFactorBitWidth = std::bit_width(static_cast<uint32_t>(VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA));
		static constexpr uint32_t BlendOpBitWidth = std::bit_width(static_cast<uint32_t>(VK_BLEND_OP_MAX));
		static constexpr uint32_t WriteMaskBitWidth = std::bit_width(static_cast<uint32_t>(VK_COLOR_COMPONENT_A_BIT));
		static constexpr uint32_t CullModeBitWidth = std::bit_width(static_cast<uint32_t>(VK_CULL_MODE_FRONT_AND_BACK));
		static constexpr uint32_t FrontFaceBitWidth = std::bit_width(static_cast<uint32_t>(VK_FRONT_FACE_CLOCKWISE));
		static constexpr uint32_t CompareOpBitWidth = std::bit_width(static_cast<uint32_t>(VK_COMPARE_OP_ALWAYS));
		static constexpr uint32_t StencilOpBitWidth = std::bit_width(static_cast<uint32_t>(VK_STENCIL_OP_DECREMENT_AND_WRAP));
		static constexpr uint32_t TopologyBitWidth = std::bit_width(static_cast<uint32_t>(VK_PRIMITIVE_TOPOLOGY_PATCH_LIST));
		static constexpr uint32_t LogicOpBitWidth = std::bit_width(static_cast<uint32_t>(VK_LOGIC_OP_SET));
		static constexpr uint32_t PolygonModeBitWidth = std::bit_width(static_cast<uint32_t>(VK_POLYGON_MODE_POINT));
		static constexpr uint32_t SampleCountBitWidth = std::bit_width(static_cast<uint32_t>(VK_SAMPLE_COUNT_8_BIT)); //In practice, only Intel supports 16 samples
		static constexpr uint32_t PatchControlPointBitWidth = std::bit_width(static_cast<uint32_t>(32)); //All devices supporting tessellation use 32 as max value
		static constexpr uint32_t StencilValueBitWidth = 8; //In practice, only 8 bit stencils are relevant
		static constexpr uint32_t SampleMaskBitWidth = SampleCountBitWidth; 
		
	public:
		static constexpr uint32_t SampleMaskBitMask = ((1ul << SampleMaskBitWidth) - 1ul);
		static constexpr uint32_t MaxAttachments = 16;
		static constexpr uint32_t MaxVertexInputs = 16;
		// Blend Logic Pseudo Code
		//if (blendEnable) {
		//	finalColor.rgb = (srcColorBlendFactor * newColor.rgb) <colorBlendOp> (dstColorBlendFactor * oldColor.rgb);
		//	finalColor.a = (srcAlphaBlendFactor * newColor.a) <alphaBlendOp> (dstAlphaBlendFactor * oldColor.a);
		//}
		//else {
		//	finalColor = newColor;
		//}
		//finalColor = finalColor & colorWriteMask;
		struct BlendAttachment {
			uint32_t blendEnable : 1 {0};
			uint32_t srcColorBlend : BlendFactorBitWidth{static_cast<uint32_t>(VK_BLEND_FACTOR_ONE)};
			uint32_t dstColorBlend : BlendFactorBitWidth{};
			uint32_t colorBlendOp : BlendOpBitWidth{ static_cast<uint32_t>(VK_BLEND_OP_ADD) };
			uint32_t srcAlphaBlend : BlendFactorBitWidth{ static_cast<uint32_t>(VK_BLEND_FACTOR_ONE) };
			uint32_t dstAlphaBlend : BlendFactorBitWidth{};
			uint32_t alphaBlendOp : BlendOpBitWidth{};
			uint32_t colorWriteMask : WriteMaskBitWidth{ static_cast<uint32_t>(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT) };
		};
		static_assert(BlendAttachment{}.colorWriteMask == (static_cast<uint32_t>(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT)));
		enum class DynamicState : uint32_t {
			Viewport,
			Scissor,
			LineWidth,
			DepthBias,
			BlendConstants,
			DepthBounds,
			StencilCompareMask,
			StencilWriteMask,
			StencilReference,
			CullMode,
			FrontFace,
			PrimitiveTopology,
			ViewportWithCount,
			ScissorWithCount,
			VertexInputBindingStride,
			DepthTestEnable,
			DepthWriteEnable,
			DepthCompareOp,
			DepthBoundsTestEnable,
			StencilTestEnable,
			StencilOp,
			RasterizerDiscardEnable,
			DepthBiasEnable,
			PrimitiveRestartEnable,
			ViewportWScaling,
			DiscardRectangle,
			DiscardRectangleEnable,
			DiscardRectangleMode,
			SampleLocations,
			ViewportShadingRatePalette,
			ViewportShadingCoarseSampleOrder,
			ExclusiveScissorEnable,
			ExclusiveScissor,
			FragmentShadingRate,
			VertexInput,
			PatchControlPoints,
			LogicOp,
			ColorWriteEnable,
			DepthClampEnable,
			PolgyonMode,
			RasterizationSamples,
			SampleMask,
			AlphaToCoverageEnable,
			AlphaToOneEnable,
			LogicOpEnable,
			ColorBlendEnable,
			ColorBlendEquation,
			ColorWriteMask,
			TesselationDomainOrigin,
			RasterizationStream,
			ConservativeRasterizationMode,
			ExtraPrimitiveOverestimationSize,
			DepthClipEnable,
			SampleLocationsEnable,
			ColorBlendAdvanced,
			ProvokingVertexMode,
			LineRasterizationMode,
			LineStippleEnable,
			DepthClipNegativeOneToOne, //OGL emulation primarely
			ViewportWScalingEnable, //NV only, not interesting 
			ViewportSwizzle, //NV only, not interesting 
			CoverageToColorEnable, //NV Only, not interesting 
			CoverageToColorLocation, //NV Only, not interesting 
			CoverageModulationMode, //NV Only, not interesting 
			CoverageModulationTableEnable, //NV Only, not interesting 
			CoverageModulationTable, //NV Only, not interesting 
			ShadingRateImageEnable, //NV Only, but interesting
			RepresentativeFragmentTestEnable, // NV only, not interesting 
			CoverageReductionMode, //NV only, not interesting 
			AttachmentFeedbackLoopEnable,
			LineStipple,						
		};
		//using DynamicStates = magic_enum::containers::bitset<DynamicState>; // Needs to be less than 64 states to 
		using DynamicStates = nyan::bitset<magic_enum::enum_count<DynamicState>(), DynamicState>; // Needs to be less than 64 states to 

		struct PipelineState {
			DynamicStates dynamicState{};

			uint8_t stencilFrontCompareMask : StencilValueBitWidth{ 0 };
			uint8_t stencilFrontWriteMask : StencilValueBitWidth{ 0 };
			uint8_t stencilFrontReference : StencilValueBitWidth{ 0 };
			uint8_t stencilBackCompareMask : StencilValueBitWidth{ 0 };
			uint8_t stencilBackWriteMask : StencilValueBitWidth{ 0 };
			uint8_t stencilBackReference : StencilValueBitWidth{ 0 };

			uint8_t depthWriteEnable : 1 {VK_FALSE};
			uint8_t depthTestEnable : 1 {VK_FALSE};
			uint8_t depthBiasEnable : 1 {VK_FALSE};
			uint8_t depthBoundsTestEnable : 1 {VK_FALSE};
			uint8_t depthClampEnable : 1 {VK_FALSE};
			uint8_t stencilTestEnable : 1 {VK_FALSE};
			uint8_t primitiveRestartEnable : 1 {VK_FALSE};
			uint8_t rasterizerDiscardEnable : 1 {VK_FALSE};

			uint32_t depthCompareOp : CompareOpBitWidth{static_cast<uint32_t>(VK_COMPARE_OP_GREATER_OR_EQUAL)}; //GE for inverse Z

			uint32_t stencilFrontFailOp : StencilOpBitWidth{ 0 };
			uint32_t stencilFrontPassOp : StencilOpBitWidth{ 0 };
			uint32_t stencilFrontDepthFailOp : StencilOpBitWidth{ 0 };
			uint32_t stencilFrontCompareOp : CompareOpBitWidth{ 0 };
			uint32_t stencilBackFailOp : StencilOpBitWidth{ 0 };
			uint32_t stencilBackPassOp : StencilOpBitWidth{ 0 };
			uint32_t stencilBackDepthFailOp : StencilOpBitWidth{ 0 };
			uint32_t stencilBackCompareOp : CompareOpBitWidth{ 0 };


			uint32_t cullMode : CullModeBitWidth{ static_cast<uint32_t>(VK_CULL_MODE_BACK_BIT) };
			uint32_t frontFace : FrontFaceBitWidth{static_cast<uint32_t>(VK_FRONT_FACE_COUNTER_CLOCKWISE)};
			uint32_t primitiveTopology : TopologyBitWidth{static_cast<uint32_t>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)};

			uint32_t polygonMode : PolygonModeBitWidth{ static_cast<uint32_t>(VK_POLYGON_MODE_FILL) };

			uint32_t rasterizationSampleCount : SampleCountBitWidth{ static_cast<uint32_t>(VK_SAMPLE_COUNT_1_BIT) };
			uint32_t alphaToCoverageEnable : 1 {0};
			uint32_t alphaToOneEnable : 1 {0};
			uint32_t sampleShadingEnable : 1 {0};
			uint32_t logicOpEnable : 1 {0};
			uint32_t logicOp : LogicOpBitWidth{ 0 };
			uint32_t patchControlPoints : PatchControlPointBitWidth{ 0};
			uint32_t sampleMask : SampleMaskBitWidth{ SampleMaskBitMask & std::numeric_limits<uint32_t>::max() };
			float minSampleShading{ 0.f };
			float depthBiasConstantFactor{ 0.f };
			float depthBiasClamp{ 0.f };
			float depthBiasSlopeFactor{ 0.f };
			float lineWidth{ 0.f };
			float minDepthBounds{ 0.f };
			float maxDepthBounds{ 0.f };
			std::array<float, 4> blendConstants{ 0.f, 0.f, 0.f, 0.f };

			struct Viewport { //No weird shenanigans allowed, and default is full scissor, everything else must be set dynamically
				uint16_t width{ 0 };
				uint16_t height{ 0 };
			} viewport;

			std::array<BlendAttachment, MaxAttachments> blendAttachments{};

		};
		static_assert(PipelineState{}.sampleMask == (SampleMaskBitMask & std::numeric_limits<uint32_t>::max()));

		//static constexpr PipelineState defaultPipelineState{
		//	//.dynamicState{},
		//	//.stencilFrontCompareMask { 0 },
		//	//.stencilFrontWriteMask { 0 },
		//	//.stencilFrontReference { 0 },
		//	//.stencilBackCompareMask { 0 },
		//	//.stencilBackWriteMask { 0 },
		//	//.stencilBackReference { 0 },
		//	//.depthWriteEnable {VK_FALSE},
		//	//.depthTestEnable {VK_FALSE},
		//	//.depthBiasEnable {VK_FALSE},
		//	//.depthBoundsTestEnable {VK_FALSE},
		//	//.depthClampEnable {VK_FALSE},
		//	//.stencilTestEnable {VK_FALSE},
		//	//.primitiveRestartEnable {VK_FALSE},
		//	//.rasterizerDiscardEnable {VK_FALSE},
		//	//.depthCompareOp {static_cast<uint32_t>(VK_COMPARE_OP_GREATER_OR_EQUAL)},
		//	//.stencilFrontFailOp { 0 },
		//	//.stencilFrontPassOp { 0 },
		//	//.stencilFrontDepthFailOp { 0 },
		//	//.stencilFrontCompareOp { 0 },
		//	//.stencilBackFailOp { 0 },
		//	//.stencilBackPassOp { 0 },
		//	//.stencilBackDepthFailOp { 0 },
		//	//.stencilBackCompareOp { 0 },
		//	//.cullMode { static_cast<uint32_t>(VK_CULL_MODE_BACK_BIT) },
		//	//.frontFace {static_cast<uint32_t>(VK_FRONT_FACE_COUNTER_CLOCKWISE)},
		//	//.primitiveTopology {static_cast<uint32_t>(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)},
		//	//.polygonMode { static_cast<uint32_t>(VK_POLYGON_MODE_FILL) },
		//	//.rasterizationSampleCount { static_cast<uint32_t>(VK_SAMPLE_COUNT_1_BIT) },
		//	//.alphaToCoverageEnable {0},
		//	//.alphaToOneEnable {0},
		//	//.sampleShadingEnable {0},
		//	//.logicOpEnable {0},
		//	//.logicOp { 0 },
		//	//.patchControlPoints { 0},
		//	//.sampleMask { std::numeric_limits<uint32_t>::max() },
		//	//.minSampleShading{ 0.f },
		//	//.depthBiasConstantFactor{ 0.f },
		//	//.depthBiasClamp{ 0.f },
		//	//.depthBiasSlopeFactor{ 0.f },
		//	//.lineWidth{ 0.f },
		//	//.minDepthBounds{ 0.f },
		//	//.maxDepthBounds{ 0.f },
		//	//.blendConstants{ 0.f, 0.f, 0.f, 0.f },
		//	//.viewport{
		//	//	.width{ 0 },
		//	//	.height{ 0 },
		//	//},
		//	//.blendAttachments{},
		//};

		struct RenderingCreateInfo {
			uint32_t viewMask{ 0 }; //Multiview, currently no intentions to support it
			std::array<RenderFormat, MaxAttachments> colorAttachmentFormats{ RenderFormat::UNDEFINED }; //Choose to ignore the higher formats
			uint8_t colorAttachmentCount{ 0 };
			DepthStencilFormat depthAttachmentFormat{ DepthStencilFormat::UNDEFINED };
			DepthStencilFormat stencilAttachmentFormat{ DepthStencilFormat::UNDEFINED };
		};
		struct RenderPassInfo {
			const RenderPass& renderPass;
			uint32_t subpass{ 0 };
		};
		using RenderInfo = std::variant<RenderingCreateInfo, RenderPassInfo>;
	protected:
		GraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept;
	public:
		[[nodiscard]] const PipelineState& get_pipeline_state() const noexcept;
	
	protected:
		PipelineState m_pipelineState;
	};

	class MeshShaderGraphicsPipeline : public GraphicsPipeline
	{
	public:
		struct Parameters {
			PipelineState pipelineState;
			RenderInfo renderInfo;
			const ShaderInstance& meshShader;
			const ShaderInstance* taskShader{ nullptr };
			const ShaderInstance& fragmentShader;
		};
	public:
		[[nodiscard("must handle potential error")]] static std::expected<MeshShaderGraphicsPipeline, Error> create(const LogicalDevice& device, const PipelineLayout& layout, const Parameters& params, PipelineCache* pipelineCache) noexcept;
	private:
		MeshShaderGraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept;
	};

	class VertexShaderGraphicsPipeline : public GraphicsPipeline
	{
	public:
		struct VertexInput {
			uint8_t vertexInputCount{ 0 };
			std::array<VertexFormat, MaxVertexInputs> vertexInputFormats{ VertexFormat::UNDEFINED }; 
		};
		struct Parameters {
			VertexInput vertexInput;
			PipelineState pipelineState;
			RenderInfo renderInfo;
			const ShaderInstance& vertexShader;
			const ShaderInstance& fragmentShader;
			const ShaderInstance* geometryShader{ nullptr };
			const ShaderInstance* tessellationControlShader{ nullptr };
			const ShaderInstance* tessellationEvaluationShader{ nullptr };
		};
	public:
		[[nodiscard("must handle potential error")]] static std::expected<VertexShaderGraphicsPipeline, Error> create(const LogicalDevice& device, const PipelineLayout& layout, const Parameters& params, PipelineCache* pipelineCache) noexcept;
	private:
		VertexShaderGraphicsPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle, PipelineState pipelineState) noexcept;
	};

	class ComputePipeline : public Pipeline
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<ComputePipeline, Error> create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, const ShaderInstance& computeShader, PipelineCache* pipelineCache) noexcept;
	private:
		ComputePipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle) noexcept;
	};

	class RayTracingPipeline : public Pipeline
	{
	public:
		enum class DynamicState {
			RayTracingPipelineStackSize
		};
		using DynamicStates = magic_enum::containers::bitset<DynamicState>;

		[[nodiscard("must handle potential error")]] static std::expected<RayTracingPipeline, Error> create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, PipelineCache* pipelineCache) noexcept;

	private:
		RayTracingPipeline(const LogicalDeviceWrapper& device, const PipelineLayout& layout, VkPipeline handle) noexcept;

	};

}
