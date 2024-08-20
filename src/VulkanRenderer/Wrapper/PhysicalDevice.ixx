module;

#include <array>
#include <expected>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "volk.h"

export module NYANVulkanWrapper:PhysicalDevice;
import :Error;
import :Queue;

export namespace nyan::vulkan::wrapper
{
	class PhysicalDevice
	{
	public:
		struct Extensions {
			uint32_t swapchain : 1 { 0 };
			uint32_t swapchainMaintenance1 : 1 { 0 };
			uint32_t fullscreenExclusive : 1 { 0 };
			uint32_t debugMarker : 1 { 0 };
			uint32_t accelerationStructure : 1 { 0 };
			uint32_t rayTracingPipeline : 1 { 0 };
			uint32_t rayQuery : 1 { 0 };
			uint32_t pipelineLibrary : 1 { 0 };
			uint32_t deferredHostOperations : 1 { 0 };
			uint32_t performanceQuery : 1 { 0 };
			uint32_t vertexInputDynamicState : 1 { 0 };
			uint32_t meshShader : 1 { 0 };
			uint32_t atomicFloats : 1 { 0 };
			uint32_t presentId : 1 { 0 };
			uint32_t presentWait : 1 { 0 };
			uint32_t pushDescriptors : 1 { 0 };
			uint32_t externalMemoryHost : 1 { 0 };
			uint32_t descriptorBuffer : 1 { 0 };
			uint32_t hostImageCopy : 1 {0};
			uint32_t dynamicRenderingLocalRead : 1 {0};
			uint32_t maintenance5 : 1 {0};
			//Promoted to Vulkan 1.3
			uint32_t copyCommands : 1 { 0 };
			uint32_t dynamicRendering : 1 { 0 };
			uint32_t formatFeatureFlags2 : 1 { 0 };
			uint32_t maintenance4 : 1 { 0 };
			uint32_t shaderIntegerDotProduct : 1 { 0 };
			uint32_t shaderNonSemanticInfo : 1 { 0 };
			uint32_t shaderTerminateInvocation : 1 { 0 };
			uint32_t synchronization2 : 1 { 0 };
			uint32_t zeroInitializeWorkgroupMemory : 1 { 0 };
			uint32_t formats4444 : 1 { 0 };
			uint32_t extendedDynamicState : 1 { 0 };
			uint32_t extendedDynamicState2 : 1 { 0 };
			uint32_t imageRobustness : 1 { 0 };
			uint32_t inlineUniformBlock : 1 { 0 };
			uint32_t pipelineCreationCacheControl : 1 { 0 };
			uint32_t pipelineCreationFeedback : 1 { 0 };
			uint32_t privateData : 1 { 0 };
			uint32_t shaderDemoteToHelperInvocation : 1 { 0 };
			uint32_t subgroupSizeControl : 1 { 0 };
			uint32_t texelBufferAlignment : 1 { 0 };
			uint32_t textureCompressionAstcHdr : 1 { 0 };
			uint32_t toolingInfo : 1 { 0 };
			uint32_t ycbcr2plane444Formats : 1 { 0 };
			//Promoted to Vulkan 1.2
			uint32_t bit8Storage : 1 { 0 };
			uint32_t bufferDeviceAddress : 1 { 0 };
			uint32_t createRenderpass2 : 1 { 0 };
			uint32_t depthStencilResolve : 1 { 0 };
			uint32_t drawIndirectCount : 1 { 0 };
			uint32_t driverProperties : 1 { 0 };
			uint32_t imageFormatList : 1 { 0 };
			uint32_t imagelessFramebuffer : 1 { 0 };
			uint32_t samplerMirrorClampToEdge : 1 { 0 };
			uint32_t separateDepthStencilLayouts : 1 { 0 };
			uint32_t shaderAtomicInt64 : 1 { 0 };
			uint32_t shaderFloat16Int8 : 1 { 0 };
			uint32_t shaderFloatControls : 1 { 0 };
			uint32_t shaderSubgroupExtendedTypes : 1 { 0 };
			uint32_t spirv14 : 1 { 0 };
			uint32_t timelineSemaphore : 1 { 0 };
			uint32_t uniformBufferStandardLayout : 1 { 0 };
			uint32_t vulkanMemoryModel : 1 { 0 };
			uint32_t descriptorIndexing : 1 { 0 };
			uint32_t hostQueryReset : 1 { 0 };
			uint32_t samplerFilterMinmax : 1 { 0 };
			uint32_t scalarBlockLayout : 1 { 0 };
			uint32_t separateStencilUsage : 1 { 0 };
			uint32_t shaderViewportIndexLayer : 1 { 0 };

			constexpr static Extensions vulkan12_core() noexcept
			{
				return {
					.bit8Storage  {1},
					.bufferDeviceAddress  {1},
					.createRenderpass2  {1},
					.depthStencilResolve  {1},
					.drawIndirectCount  {1},
					.driverProperties  {1},
					.imageFormatList  {1},
					.imagelessFramebuffer  {1},
					.samplerMirrorClampToEdge  {1},
					.separateDepthStencilLayouts  {1},
					.shaderAtomicInt64  {1},
					.shaderFloat16Int8  {1},
					.shaderFloatControls  {1},
					.shaderSubgroupExtendedTypes  {1},
					.spirv14  {1},
					.timelineSemaphore  {1},
					.uniformBufferStandardLayout  {1},
					.vulkanMemoryModel  {1},
					.descriptorIndexing  {1},
					.hostQueryReset  {1},
					.samplerFilterMinmax  {1},
					.scalarBlockLayout  {1},
					.separateStencilUsage  {1},
					.shaderViewportIndexLayer  {1}
				};
			}
			constexpr static Extensions vulkan13_core() noexcept
			{
				return vulkan12_core() | Extensions{
					.copyCommands { 1 },
					.dynamicRendering { 1 },
					.formatFeatureFlags2 { 1 },
					.maintenance4 { 1 },
					.shaderIntegerDotProduct { 1 },
					.shaderNonSemanticInfo { 1 },
					.shaderTerminateInvocation { 1 },
					.synchronization2 { 1 },
					.zeroInitializeWorkgroupMemory { 1 },
					.formats4444 { 1 },
					.extendedDynamicState { 1 },
					.extendedDynamicState2 { 1 },
					.imageRobustness { 1 },
					.inlineUniformBlock { 1 },
					.pipelineCreationCacheControl { 1 },
					.pipelineCreationFeedback { 1 },
					.privateData { 1 },
					.shaderDemoteToHelperInvocation { 1 },
					.subgroupSizeControl { 1 },
					.texelBufferAlignment { 1 },
					.textureCompressionAstcHdr { 1 },
					.toolingInfo { 1 },
					.ycbcr2plane444Formats { 1 }
				};
			}

			bool constexpr operator==(const Extensions& other) const noexcept = default;
			Extensions operator~() const noexcept
			{
				return {
					.swapchain  {~swapchain },
					.swapchainMaintenance1 {~swapchainMaintenance1},
					.fullscreenExclusive  {~fullscreenExclusive },
					.debugMarker  {~debugMarker },
					.accelerationStructure  {~accelerationStructure },
					.rayTracingPipeline  {~rayTracingPipeline },
					.rayQuery  {~rayQuery },
					.pipelineLibrary  {~pipelineLibrary },
					.deferredHostOperations  {~deferredHostOperations },
					.performanceQuery  {~performanceQuery },
					.vertexInputDynamicState  {~vertexInputDynamicState },
					.meshShader  {~meshShader },
					.atomicFloats  {~atomicFloats },
					.presentId  {~presentId },
					.presentWait  {~presentWait },
					.pushDescriptors {~pushDescriptors},
					.externalMemoryHost {~externalMemoryHost},
					.descriptorBuffer {~descriptorBuffer},
					.hostImageCopy {~hostImageCopy},
					.dynamicRenderingLocalRead {~dynamicRenderingLocalRead},
					.maintenance5 {~maintenance5},
					.copyCommands  {~copyCommands },
					.dynamicRendering  {~dynamicRendering },
					.formatFeatureFlags2  {~formatFeatureFlags2 },
					.maintenance4  {~maintenance4 },
					.shaderIntegerDotProduct  {~shaderIntegerDotProduct },
					.shaderNonSemanticInfo  {~shaderNonSemanticInfo },
					.shaderTerminateInvocation  {~shaderTerminateInvocation },
					.synchronization2  {~synchronization2 },
					.zeroInitializeWorkgroupMemory  {~zeroInitializeWorkgroupMemory },
					.formats4444  {~formats4444 },
					.extendedDynamicState  {~extendedDynamicState },
					.extendedDynamicState2  {~extendedDynamicState2 },
					.imageRobustness  {~imageRobustness },
					.inlineUniformBlock  {~inlineUniformBlock },
					.pipelineCreationCacheControl  {~pipelineCreationCacheControl },
					.pipelineCreationFeedback  {~pipelineCreationFeedback },
					.privateData  {~privateData },
					.shaderDemoteToHelperInvocation  {~shaderDemoteToHelperInvocation },
					.subgroupSizeControl  {~subgroupSizeControl },
					.texelBufferAlignment  {~texelBufferAlignment },
					.textureCompressionAstcHdr  {~textureCompressionAstcHdr },
					.toolingInfo  {~toolingInfo },
					.ycbcr2plane444Formats  {~ycbcr2plane444Formats },
					.bit8Storage  {~bit8Storage },
					.bufferDeviceAddress  {~bufferDeviceAddress },
					.createRenderpass2  {~createRenderpass2 },
					.depthStencilResolve  {~depthStencilResolve },
					.drawIndirectCount  {~drawIndirectCount },
					.driverProperties  {~driverProperties },
					.imageFormatList  {~imageFormatList },
					.imagelessFramebuffer  {~imagelessFramebuffer },
					.samplerMirrorClampToEdge  {~samplerMirrorClampToEdge },
					.separateDepthStencilLayouts  {~separateDepthStencilLayouts },
					.shaderAtomicInt64  {~shaderAtomicInt64 },
					.shaderFloat16Int8  {~shaderFloat16Int8 },
					.shaderFloatControls  {~shaderFloatControls },
					.shaderSubgroupExtendedTypes  {~shaderSubgroupExtendedTypes },
					.spirv14  {~spirv14 },
					.timelineSemaphore  {~timelineSemaphore },
					.uniformBufferStandardLayout  {~uniformBufferStandardLayout },
					.vulkanMemoryModel  {~vulkanMemoryModel },
					.descriptorIndexing  {~descriptorIndexing },
					.hostQueryReset  {~hostQueryReset },
					.samplerFilterMinmax  {~samplerFilterMinmax },
					.scalarBlockLayout  {~scalarBlockLayout },
					.separateStencilUsage  {~separateStencilUsage },
					.shaderViewportIndexLayer  {~shaderViewportIndexLayer }
				};
			}
			friend constexpr Extensions operator&(const Extensions& lhs, const Extensions& rhs) noexcept
			{
				return Extensions {
					.swapchain  {lhs.swapchain & rhs.swapchain },
					.swapchainMaintenance1 {lhs.swapchainMaintenance1 & rhs.swapchainMaintenance1},
					.fullscreenExclusive  {lhs.fullscreenExclusive& rhs.fullscreenExclusive },
					.debugMarker  {lhs.debugMarker& rhs.debugMarker },
					.accelerationStructure  {lhs.accelerationStructure& rhs.accelerationStructure },
					.rayTracingPipeline  {lhs.rayTracingPipeline& rhs.rayTracingPipeline },
					.rayQuery  {lhs.rayQuery& rhs.rayQuery },
					.pipelineLibrary  {lhs.pipelineLibrary& rhs.pipelineLibrary },
					.deferredHostOperations  {lhs.deferredHostOperations& rhs.deferredHostOperations },
					.performanceQuery  {lhs.performanceQuery& rhs.performanceQuery },
					.vertexInputDynamicState  {lhs.vertexInputDynamicState& rhs.vertexInputDynamicState },
					.meshShader  {lhs.meshShader& rhs.meshShader },
					.atomicFloats  {lhs.atomicFloats& rhs.atomicFloats },
					.presentId  {lhs.presentId& rhs.presentId },
					.presentWait  {lhs.presentWait& rhs.presentWait },
					.pushDescriptors {lhs.pushDescriptors& rhs.pushDescriptors },
					.externalMemoryHost {lhs.externalMemoryHost& rhs.externalMemoryHost },
					.descriptorBuffer {lhs.descriptorBuffer & rhs.descriptorBuffer},
					.hostImageCopy {lhs.hostImageCopy & rhs.hostImageCopy},
					.dynamicRenderingLocalRead {lhs.dynamicRenderingLocalRead & rhs.dynamicRenderingLocalRead},
					.maintenance5 {lhs.maintenance5 & rhs.maintenance5},
					.copyCommands  {lhs.copyCommands& rhs.copyCommands },
					.dynamicRendering  {lhs.dynamicRendering& rhs.dynamicRendering },
					.formatFeatureFlags2  {lhs.formatFeatureFlags2& rhs.formatFeatureFlags2 },
					.maintenance4  {lhs.maintenance4& rhs.maintenance4 },
					.shaderIntegerDotProduct  {lhs.shaderIntegerDotProduct& rhs.shaderIntegerDotProduct },
					.shaderNonSemanticInfo  {lhs.shaderNonSemanticInfo& rhs.shaderNonSemanticInfo },
					.shaderTerminateInvocation  {lhs.shaderTerminateInvocation& rhs.shaderTerminateInvocation },
					.synchronization2  {lhs.synchronization2& rhs.synchronization2 },
					.zeroInitializeWorkgroupMemory  {lhs.zeroInitializeWorkgroupMemory& rhs.zeroInitializeWorkgroupMemory },
					.formats4444  {lhs.formats4444& rhs.formats4444 },
					.extendedDynamicState  {lhs.extendedDynamicState& rhs.extendedDynamicState },
					.extendedDynamicState2  {lhs.extendedDynamicState2& rhs.extendedDynamicState2 },
					.imageRobustness  {lhs.imageRobustness& rhs.imageRobustness },
					.inlineUniformBlock  {lhs.inlineUniformBlock& rhs.inlineUniformBlock },
					.pipelineCreationCacheControl  {lhs.pipelineCreationCacheControl& rhs.pipelineCreationCacheControl },
					.pipelineCreationFeedback  {lhs.pipelineCreationFeedback& rhs.pipelineCreationFeedback },
					.privateData  {lhs.privateData& rhs.privateData },
					.shaderDemoteToHelperInvocation  {lhs.shaderDemoteToHelperInvocation& rhs.shaderDemoteToHelperInvocation },
					.subgroupSizeControl  {lhs.subgroupSizeControl& rhs.subgroupSizeControl },
					.texelBufferAlignment  {lhs.texelBufferAlignment& rhs.texelBufferAlignment },
					.textureCompressionAstcHdr  {lhs.textureCompressionAstcHdr& rhs.textureCompressionAstcHdr },
					.toolingInfo  {lhs.toolingInfo& rhs.toolingInfo },
					.ycbcr2plane444Formats  {lhs.ycbcr2plane444Formats& rhs.ycbcr2plane444Formats },
					.bit8Storage  {lhs.bit8Storage& rhs.bit8Storage },
					.bufferDeviceAddress  {lhs.bufferDeviceAddress& rhs.bufferDeviceAddress },
					.createRenderpass2  {lhs.createRenderpass2& rhs.createRenderpass2 },
					.depthStencilResolve  {lhs.depthStencilResolve& rhs.depthStencilResolve },
					.drawIndirectCount  {lhs.drawIndirectCount& rhs.drawIndirectCount },
					.driverProperties  {lhs.driverProperties& rhs.driverProperties },
					.imageFormatList  {lhs.imageFormatList& rhs.imageFormatList },
					.imagelessFramebuffer  {lhs.imagelessFramebuffer& rhs.imagelessFramebuffer },
					.samplerMirrorClampToEdge  {lhs.samplerMirrorClampToEdge& rhs.samplerMirrorClampToEdge },
					.separateDepthStencilLayouts  {lhs.separateDepthStencilLayouts& rhs.separateDepthStencilLayouts },
					.shaderAtomicInt64  {lhs.shaderAtomicInt64& rhs.shaderAtomicInt64 },
					.shaderFloat16Int8  {lhs.shaderFloat16Int8& rhs.shaderFloat16Int8 },
					.shaderFloatControls  {lhs.shaderFloatControls& rhs.shaderFloatControls },
					.shaderSubgroupExtendedTypes  {lhs.shaderSubgroupExtendedTypes& rhs.shaderSubgroupExtendedTypes },
					.spirv14  {lhs.spirv14& rhs.spirv14 },
					.timelineSemaphore  {lhs.timelineSemaphore& rhs.timelineSemaphore },
					.uniformBufferStandardLayout  {lhs.uniformBufferStandardLayout& rhs.uniformBufferStandardLayout },
					.vulkanMemoryModel  {lhs.vulkanMemoryModel& rhs.vulkanMemoryModel },
					.descriptorIndexing  {lhs.descriptorIndexing& rhs.descriptorIndexing },
					.hostQueryReset  {lhs.hostQueryReset& rhs.hostQueryReset },
					.samplerFilterMinmax  {lhs.samplerFilterMinmax& rhs.samplerFilterMinmax },
					.scalarBlockLayout  {lhs.scalarBlockLayout& rhs.scalarBlockLayout },
					.separateStencilUsage  {lhs.separateStencilUsage& rhs.separateStencilUsage },
					.shaderViewportIndexLayer  {lhs.shaderViewportIndexLayer & rhs.shaderViewportIndexLayer },
				};
			}
			friend constexpr Extensions operator|(const Extensions& lhs, const Extensions& rhs) noexcept
			{
				return Extensions {
					.swapchain  {lhs.swapchain | rhs.swapchain},
					.swapchainMaintenance1  {lhs.swapchainMaintenance1 | rhs.swapchainMaintenance1},
					.fullscreenExclusive  {lhs.fullscreenExclusive | rhs.fullscreenExclusive},
					.debugMarker  {lhs.debugMarker | rhs.debugMarker},
					.accelerationStructure  {lhs.accelerationStructure | rhs.accelerationStructure},
					.rayTracingPipeline  {lhs.rayTracingPipeline | rhs.rayTracingPipeline},
					.rayQuery  {lhs.rayQuery | rhs.rayQuery},
					.pipelineLibrary  {lhs.pipelineLibrary | rhs.pipelineLibrary},
					.deferredHostOperations  {lhs.deferredHostOperations | rhs.deferredHostOperations},
					.performanceQuery  {lhs.performanceQuery | rhs.performanceQuery},
					.vertexInputDynamicState  {lhs.vertexInputDynamicState | rhs.vertexInputDynamicState},
					.meshShader  {lhs.meshShader | rhs.meshShader},
					.atomicFloats  {lhs.atomicFloats | rhs.atomicFloats},
					.presentId  {lhs.presentId | rhs.presentId},
					.presentWait  {lhs.presentWait | rhs.presentWait},
					.pushDescriptors {lhs.pushDescriptors | rhs.pushDescriptors},
					.externalMemoryHost {lhs.externalMemoryHost | rhs.externalMemoryHost},
					.descriptorBuffer {lhs.descriptorBuffer | rhs.descriptorBuffer},
					.hostImageCopy {lhs.hostImageCopy | rhs.hostImageCopy},
					.dynamicRenderingLocalRead {lhs.dynamicRenderingLocalRead | rhs.dynamicRenderingLocalRead},
					.maintenance5 {lhs.maintenance5 | rhs.maintenance5},
					.copyCommands  {lhs.copyCommands | rhs.copyCommands},
					.dynamicRendering  {lhs.dynamicRendering | rhs.dynamicRendering},
					.formatFeatureFlags2  {lhs.formatFeatureFlags2 | rhs.formatFeatureFlags2},
					.maintenance4  {lhs.maintenance4 | rhs.maintenance4},
					.shaderIntegerDotProduct  {lhs.shaderIntegerDotProduct | rhs.shaderIntegerDotProduct},
					.shaderNonSemanticInfo  {lhs.shaderNonSemanticInfo | rhs.shaderNonSemanticInfo},
					.shaderTerminateInvocation  {lhs.shaderTerminateInvocation | rhs.shaderTerminateInvocation},
					.synchronization2  {lhs.synchronization2 | rhs.synchronization2},
					.zeroInitializeWorkgroupMemory  {lhs.zeroInitializeWorkgroupMemory | rhs.zeroInitializeWorkgroupMemory},
					.formats4444  {lhs.formats4444 | rhs.formats4444},
					.extendedDynamicState  {lhs.extendedDynamicState | rhs.extendedDynamicState},
					.extendedDynamicState2  {lhs.extendedDynamicState2 | rhs.extendedDynamicState2},
					.imageRobustness  {lhs.imageRobustness | rhs.imageRobustness},
					.inlineUniformBlock  {lhs.inlineUniformBlock | rhs.inlineUniformBlock},
					.pipelineCreationCacheControl  {lhs.pipelineCreationCacheControl | rhs.pipelineCreationCacheControl},
					.pipelineCreationFeedback  {lhs.pipelineCreationFeedback | rhs.pipelineCreationFeedback},
					.privateData  {lhs.privateData | rhs.privateData},
					.shaderDemoteToHelperInvocation  {lhs.shaderDemoteToHelperInvocation | rhs.shaderDemoteToHelperInvocation},
					.subgroupSizeControl  {lhs.subgroupSizeControl | rhs.subgroupSizeControl},
					.texelBufferAlignment  {lhs.texelBufferAlignment | rhs.texelBufferAlignment},
					.textureCompressionAstcHdr  {lhs.textureCompressionAstcHdr | rhs.textureCompressionAstcHdr},
					.toolingInfo  {lhs.toolingInfo | rhs.toolingInfo},
					.ycbcr2plane444Formats  {lhs.ycbcr2plane444Formats | rhs.ycbcr2plane444Formats},
					.bit8Storage  {lhs.bit8Storage | rhs.bit8Storage},
					.bufferDeviceAddress  {lhs.bufferDeviceAddress | rhs.bufferDeviceAddress},
					.createRenderpass2  {lhs.createRenderpass2 | rhs.createRenderpass2},
					.depthStencilResolve  {lhs.depthStencilResolve | rhs.depthStencilResolve},
					.drawIndirectCount  {lhs.drawIndirectCount | rhs.drawIndirectCount},
					.driverProperties  {lhs.driverProperties | rhs.driverProperties},
					.imageFormatList  {lhs.imageFormatList | rhs.imageFormatList},
					.imagelessFramebuffer  {lhs.imagelessFramebuffer | rhs.imagelessFramebuffer},
					.samplerMirrorClampToEdge  {lhs.samplerMirrorClampToEdge | rhs.samplerMirrorClampToEdge},
					.separateDepthStencilLayouts  {lhs.separateDepthStencilLayouts | rhs.separateDepthStencilLayouts},
					.shaderAtomicInt64  {lhs.shaderAtomicInt64 | rhs.shaderAtomicInt64},
					.shaderFloat16Int8  {lhs.shaderFloat16Int8 | rhs.shaderFloat16Int8},
					.shaderFloatControls  {lhs.shaderFloatControls | rhs.shaderFloatControls},
					.shaderSubgroupExtendedTypes  {lhs.shaderSubgroupExtendedTypes | rhs.shaderSubgroupExtendedTypes},
					.spirv14  {lhs.spirv14 | rhs.spirv14},
					.timelineSemaphore  {lhs.timelineSemaphore | rhs.timelineSemaphore},
					.uniformBufferStandardLayout  {lhs.uniformBufferStandardLayout | rhs.uniformBufferStandardLayout},
					.vulkanMemoryModel  {lhs.vulkanMemoryModel | rhs.vulkanMemoryModel},
					.descriptorIndexing  {lhs.descriptorIndexing | rhs.descriptorIndexing},
					.hostQueryReset  {lhs.hostQueryReset | rhs.hostQueryReset},
					.samplerFilterMinmax  {lhs.samplerFilterMinmax | rhs.samplerFilterMinmax},
					.scalarBlockLayout  {lhs.scalarBlockLayout | rhs.scalarBlockLayout},
					.separateStencilUsage  {lhs.separateStencilUsage | rhs.separateStencilUsage},
					.shaderViewportIndexLayer  {lhs.shaderViewportIndexLayer | rhs.shaderViewportIndexLayer},
				};
			}
			[[nodiscard]] std::vector<const char*> generate_extension_list(uint32_t apiVersion) const noexcept;
			[[nodiscard]] Extensions generate_dependent_extensions(uint32_t apiVersion) const noexcept;

			[[nodiscard]] static Extensions generate_extension(std::string_view extensionName) noexcept;
		};
		enum class Type : uint32_t
		{
			Discrete,
			Integrated,
			Virtual,
			CPU,
			Other
		};
	public:

		[[nodiscard]] const VkPhysicalDeviceFeatures& get_used_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan11Features& get_vulkan11_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan12Features& get_vulkan12_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan13Features& get_vulkan13_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& get_atomic_float_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceAccelerationStructureFeaturesKHR& get_acceleration_structure_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& get_ray_tracing_pipeline_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayQueryFeaturesKHR& get_ray_query_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& get_vertex_input_dynamic_state_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMeshShaderFeaturesEXT& get_mesh_shader_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceDescriptorBufferFeaturesEXT& get_descriptor_buffer_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceHostImageCopyFeaturesEXT& get_host_image_copy_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR& get_dynamic_rendering_local_read_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMaintenance5FeaturesKHR& get_maintenance5_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT& get_swapchain_maintenance1_features() const noexcept;

		[[nodiscard]] const VkPhysicalDeviceProperties& get_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceSubgroupProperties& get_subgroup_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan11Properties& get_vulkan11_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan12Properties& get_vulkan12_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan13Properties& get_vulkan13_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMeshShaderPropertiesEXT& get_mesh_shader_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDevicePushDescriptorPropertiesKHR& get_push_descriptor_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceExternalMemoryHostPropertiesEXT& get_external_memory_host_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceDescriptorBufferPropertiesEXT& get_descriptor_buffer_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMaintenance5PropertiesKHR& get_maintenance5_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceHostImageCopyPropertiesEXT& get_host_image_copy_properties() const noexcept;
		[[nodiscard]] std::span<const VkImageLayout> get_host_image_copy_src_layouts() const noexcept;
		[[nodiscard]] std::span<const VkImageLayout> get_host_image_copy_dst_layouts() const noexcept;

		[[nodiscard]] const VkPhysicalDeviceMemoryProperties& get_memory_properties() const noexcept;

		[[nodiscard]] const bool supports_rebar() const noexcept;

		[[nodiscard]] const bool supports_bar() const noexcept;

		[[nodiscard]] const Extensions& get_available_extensions() const noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<PhysicalDevice, PhysicalDeviceCreationError> create(VkPhysicalDevice handle);

		//Must be called before using features for 
		[[nodiscard]] const VkPhysicalDeviceFeatures2& build_feature_chain(const Extensions& extensions) noexcept;

		[[nodiscard]] VkPhysicalDevice get_handle() const noexcept;

		[[nodiscard]] uint32_t get_queue_family_index(Queue::Type queueType) const noexcept;

		[[nodiscard]] Type get_type() const noexcept;

		[[nodiscard]] uint64_t get_id() const noexcept;
	private:
		explicit PhysicalDevice(VkPhysicalDevice handle, const Extensions& availableExtensions) noexcept;
		void init_type() noexcept;
		void init_memory_properties() noexcept;
		void init_queues() noexcept;
		void init_features() noexcept;
		void init_properties() noexcept;


		VkPhysicalDevice m_handle{ VK_NULL_HANDLE };
		Type m_type;

		QueueTypeArray<uint32_t> m_queueFamilyIndices{ invalidQueueFamilyIndex, invalidQueueFamilyIndex, invalidQueueFamilyIndex, invalidQueueFamilyIndex, invalidQueueFamilyIndex };

		bool m_supportsRebar{ false };
		bool m_supportsBar{ false };

		Extensions m_availableExtensions{};

		VkPhysicalDeviceFeatures2 m_features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan11Features  m_11Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		VkPhysicalDeviceVulkan12Features  m_12Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan13Features  m_13Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT m_atomicFloatFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		VkPhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
		VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT m_vertexInputDynamicStateFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT };
		VkPhysicalDeviceMeshShaderFeaturesEXT m_meshShaderFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
		VkPhysicalDevicePresentIdFeaturesKHR  m_presentIdFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR };
		VkPhysicalDevicePresentWaitFeaturesKHR   m_presentWaitFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR };
		VkPhysicalDeviceDescriptorBufferFeaturesEXT m_descriptorBufferFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_FEATURES_EXT };
		VkPhysicalDeviceHostImageCopyFeaturesEXT m_hostImageCopyFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_FEATURES_EXT };
		VkPhysicalDeviceDynamicRenderingLocalReadFeaturesKHR m_dynamicRenderingLocalReadFeatures {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_LOCAL_READ_FEATURES_KHR};
		VkPhysicalDeviceMaintenance5FeaturesKHR m_maintenance5Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_FEATURES_KHR };
		VkPhysicalDeviceSwapchainMaintenance1FeaturesEXT m_swapchainMaintenance1Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SWAPCHAIN_MAINTENANCE_1_FEATURES_EXT };


		//Promoted to Vulkan 1.3
		VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT m_pipelineCreationCacheControlFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT };
		VkPhysicalDevicePrivateDataFeaturesEXT m_privateDataFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT };
		VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT m_shaderDemoteToHelperInvocationFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT };
		VkPhysicalDeviceSubgroupSizeControlFeaturesEXT m_subgroupSizeControlFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT };
		VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT m_texelBufferAlignmentFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT };
		VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT m_textureCompressionAstchdrFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT };
		VkPhysicalDeviceImageRobustnessFeaturesEXT m_imageRobustnessFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT };
		VkPhysicalDeviceInlineUniformBlockFeaturesEXT m_inlineUniformBlockFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT };
		VkPhysicalDeviceDynamicRenderingFeaturesKHR m_dynamicRenderingFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		VkPhysicalDeviceMaintenance4FeaturesKHR m_maintenance4Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR };
		VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR m_shaderIntegerDotProductFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES_KHR };
		VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR m_shaderTerminateInvocationFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR };
		VkPhysicalDeviceSynchronization2FeaturesKHR m_synchronization2Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR m_zeroInitializeWorkgroupMemoryFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR };
		VkPhysicalDevice4444FormatsFeaturesEXT m_4444FormatsFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT };
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT m_extendedDynamicStateFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		VkPhysicalDeviceExtendedDynamicState2FeaturesEXT m_extendedDynamicState2Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT };
		VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT m_ycbcr2Plane444FormatsFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT };

		//Promoted to Vulkan 1.2
		VkPhysicalDevice8BitStorageFeaturesKHR m_8bitStorageFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR m_bufferDeviceAddressFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR };
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR m_separateDepthStencilLayoutsFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR };
		VkPhysicalDeviceShaderAtomicInt64FeaturesKHR m_shaderAtomicInt64Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR };
		VkPhysicalDeviceShaderFloat16Int8FeaturesKHR m_shaderFloat16Int8Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR };
		VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR m_shaderSubgroupExtendedTypesFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR };
		VkPhysicalDeviceTimelineSemaphoreFeaturesKHR m_timelineSemaphoreFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR };
		VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR m_uniformBufferStandardLayoutFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES_KHR };
		VkPhysicalDeviceVulkanMemoryModelFeaturesKHR m_vulkanMemoryModelFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR };
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT m_descriptorIndexingFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
		VkPhysicalDeviceHostQueryResetFeaturesEXT m_hostQueryResetFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT };
		VkPhysicalDeviceScalarBlockLayoutFeaturesEXT m_scalarBlockLayoutFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT };
		VkPhysicalDeviceImagelessFramebufferFeaturesKHR m_imagelessFramebufferFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR };

		VkPhysicalDeviceProperties2 m_properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		VkPhysicalDeviceSubgroupProperties m_subgroupProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
		VkPhysicalDeviceVulkan11Properties  m_11Properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
		VkPhysicalDeviceVulkan12Properties  m_12Properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
		VkPhysicalDeviceVulkan13Properties  m_13Properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES };
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceMeshShaderPropertiesEXT m_meshShaderProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };
		VkPhysicalDevicePushDescriptorPropertiesKHR m_pushDescriptorProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PUSH_DESCRIPTOR_PROPERTIES_KHR };
		VkPhysicalDeviceExternalMemoryHostPropertiesEXT m_externalMemoryHostProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTERNAL_MEMORY_HOST_PROPERTIES_EXT };
		VkPhysicalDeviceDescriptorBufferPropertiesEXT m_descriptorBufferProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_BUFFER_PROPERTIES_EXT };
		VkPhysicalDeviceHostImageCopyPropertiesEXT m_hostImageCopyProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_IMAGE_COPY_PROPERTIES_EXT };
		VkPhysicalDeviceMaintenance5PropertiesKHR m_maintenance5Properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_5_PROPERTIES_KHR };

		static constexpr size_t numImageLayouts = 38;
		std::array<VkImageLayout, numImageLayouts> m_hostImageCopySrcLayouts;
		std::array<VkImageLayout, numImageLayouts> m_hostImageCopyDstLayouts;

		VkPhysicalDeviceMemoryProperties m_memoryProperties{};

		//Promoted to Vulkan 1.3
		VkPhysicalDeviceSubgroupSizeControlPropertiesEXT m_subgroupSizeControlProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES };
		VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT m_texelBufferAlignmentProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES };
		VkPhysicalDeviceInlineUniformBlockPropertiesEXT m_inlineUniformBlockProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES };
		VkPhysicalDeviceMaintenance4PropertiesKHR m_maintenance4Properties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES };
		VkPhysicalDeviceShaderIntegerDotProductPropertiesKHR m_shaderIntegerDotProductProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES };

		//Promoted to Vulkan 1.2
		VkPhysicalDeviceDriverPropertiesKHR m_driverProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR };
		VkPhysicalDeviceFloatControlsPropertiesKHR m_floatControlsProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES_KHR };
		VkPhysicalDeviceTimelineSemaphorePropertiesKHR m_timelineSemaphoreProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR };
		VkPhysicalDeviceDescriptorIndexingPropertiesEXT m_descriptorIndexingProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT };
		VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT m_samplerFilterMinmaxProperties{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT };
	};
}
