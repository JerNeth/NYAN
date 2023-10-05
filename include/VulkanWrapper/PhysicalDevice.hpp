#pragma once

#include <memory>
#include <vector>
#include <optional>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"

namespace vulkan {
	class PhysicalDevice {
	public:
		struct Extensions {
			uint32_t swapchain : 1;
			uint32_t fullscreen_exclusive : 1;
			uint32_t debug_utils : 1;
			uint32_t debug_marker : 1;
			uint32_t acceleration_structure : 1;
			uint32_t ray_tracing_pipeline : 1;
			uint32_t ray_query : 1;
			uint32_t pipeline_library : 1;
			uint32_t deferred_host_operations : 1;
			uint32_t performance_query : 1;
			uint32_t vertex_input_dynamic_state : 1;
			uint32_t mesh_shader : 1;
			uint32_t atomic_floats : 1;
			uint32_t present_id : 1;
			uint32_t present_wait : 1;
			//Promoted to Vulkan 1.3
			uint32_t copyCommands : 1;
			uint32_t dynamicRendering : 1;
			uint32_t formatFeatureFlags2 : 1;
			uint32_t maintenance4 : 1;
			uint32_t shaderIntegerDotProduct : 1;
			uint32_t shaderNonSemanticInfo : 1;
			uint32_t shaderTerminateInvocation : 1;
			uint32_t synchronization2 : 1;
			uint32_t zeroInitializeWorkgroupMemory : 1;
			uint32_t formats4444 : 1;
			uint32_t extendedDynamicState : 1;
			uint32_t extendedDynamicState2 : 1;
			uint32_t imageRobustness : 1;
			uint32_t inlineUniformBlock : 1;
			uint32_t pipelineCreationCacheControl : 1;
			uint32_t pipelineCreationFeedback : 1;
			uint32_t privateData : 1;
			uint32_t shaderDemoteToHelperInvocation : 1;
			uint32_t subgroupSizeControl : 1;
			uint32_t texelBufferAlignment : 1;
			uint32_t textureCompressionAstcHdr : 1;
			uint32_t toolingInfo : 1;
			uint32_t ycbcr2plane444Formats : 1;
			//Promoted to Vulkan 1.2
			uint32_t bit8Storage : 1;
			uint32_t bufferDeviceAddress : 1;
			uint32_t createRenderpass2 : 1;
			uint32_t depthStencilResolve : 1;
			uint32_t drawIndirectCount : 1;
			uint32_t driverProperties : 1;
			uint32_t imageFormatList : 1;
			uint32_t imagelessFramebuffer : 1;
			uint32_t samplerMirrorClampToEdge : 1;
			uint32_t separateDepthStencilLayouts : 1;
			uint32_t shaderAtomicInt64 : 1;
			uint32_t shaderFloat16Int8 : 1;
			uint32_t float16Int8 : 1;
			uint32_t shaderFloatControls : 1;
			uint32_t shaderSubgroupExtendedTypes : 1;
			uint32_t spirv14 : 1;
			uint32_t timelineSemaphore : 1;
			uint32_t uniformBufferStandardLayout : 1;
			uint32_t vulkanMemoryModel : 1;
			uint32_t descriptorIndexing : 1;
			uint32_t hostQueryReset : 1;
			uint32_t samplerFilterMinmax : 1;
			uint32_t scalarBlockLayout : 1;
			uint32_t separateStencilUsage : 1;
			uint32_t shaderViewportIndexLayer : 1;

		};
	public:
		explicit PhysicalDevice(VkPhysicalDevice device);
		[[nodiscard]] operator VkPhysicalDevice() const noexcept;
		[[nodiscard]] const VkPhysicalDevice& get_handle() const noexcept;

		[[nodiscard]] bool use_extension(const char* extension) noexcept;
		[[nodiscard]] std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance);
		[[nodiscard]] std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance, const std::vector<float>& genericQueuePriorities, const std::vector<float>& computeQueuePriorities, const std::vector<float>& transferQueuePriorities);
		
		[[nodiscard]] bool supports_surface(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;
		[[nodiscard]] std::vector<VkPresentModeKHR> get_present_modes(VkSurfaceKHR surface) const;
		[[nodiscard]] std::vector<VkSurfaceFormat2KHR> get_surface_formats2(VkSurfaceKHR surface) const;
		[[nodiscard]] VkSurfaceCapabilities2KHR get_surface_capabilites2(VkSurfaceKHR surface) const;

		[[nodiscard]] const std::vector<const char*>& get_used_extensions() const noexcept;
		[[nodiscard]] const std::vector<VkQueueFamilyProperties>& get_queue_family_properties() const noexcept;
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

		[[nodiscard]] const VkPhysicalDeviceProperties& get_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceSubgroupProperties& get_subgroup_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan11Properties& get_vulkan11_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan12Properties& get_vulkan12_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan13Properties& get_vulkan13_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMeshShaderPropertiesEXT& get_mesh_shader_properties() const noexcept;

		[[nodiscard]] const Extensions& get_extensions() const noexcept;
		[[nodiscard]] std::optional<VkImageFormatProperties> get_image_format_properties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const noexcept;
	private:
		void init_queues() noexcept;
		void init_extensions() noexcept;
		void init_features() noexcept;
		void init_properties() noexcept;
		void update_feature_chain() noexcept;


		VkPhysicalDevice m_vkHandle = VK_NULL_HANDLE;
		uint32_t m_genericQueueFamily = ~0u;
		uint32_t m_transferQueueFamily = ~0u;
		uint32_t m_computeQueueFamily = ~0u;
		Extensions m_extensions{};
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties {};
		std::vector<VkExtensionProperties> m_availableExtensions {};
		std::vector<const char*> m_usedExtensions {};

		VkPhysicalDeviceFeatures2 m_features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
		VkPhysicalDeviceVulkan11Features  m_11Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES };
		VkPhysicalDeviceVulkan12Features  m_12Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES };
		VkPhysicalDeviceVulkan13Features  m_13Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES };
		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT m_atomicFloatFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT };
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
		VkPhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR };
		VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT m_vertexInputDynamicStateFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT };
		VkPhysicalDeviceMeshShaderFeaturesEXT m_meshShaderFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT };
		VkPhysicalDevicePresentIdFeaturesKHR  m_presentIdFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR };
		VkPhysicalDevicePresentWaitFeaturesKHR   m_presentWaitFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR };

		//Promoted to Vulkan 1.3
		VkPhysicalDevicePipelineCreationCacheControlFeaturesEXT m_pipelineCreationCacheControlFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PIPELINE_CREATION_CACHE_CONTROL_FEATURES_EXT };
		VkPhysicalDevicePrivateDataFeaturesEXT m_privateDataFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRIVATE_DATA_FEATURES_EXT};
		VkPhysicalDeviceShaderDemoteToHelperInvocationFeaturesEXT m_shaderDemoteToHelperInvocationFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DEMOTE_TO_HELPER_INVOCATION_FEATURES_EXT };
		VkPhysicalDeviceSubgroupSizeControlFeaturesEXT m_subgroupSizeControlFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT};
		VkPhysicalDeviceTexelBufferAlignmentFeaturesEXT m_texelBufferAlignmentFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_FEATURES_EXT };
		VkPhysicalDeviceTextureCompressionASTCHDRFeaturesEXT m_textureCompressionAstchdrFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXTURE_COMPRESSION_ASTC_HDR_FEATURES_EXT };
		VkPhysicalDeviceImageRobustnessFeaturesEXT m_imageRobustnessFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGE_ROBUSTNESS_FEATURES_EXT };
		VkPhysicalDeviceInlineUniformBlockFeaturesEXT m_inlineUniformBlockFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_FEATURES_EXT };
		VkPhysicalDeviceDynamicRenderingFeaturesKHR m_dynamicRenderingFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR };
		VkPhysicalDeviceMaintenance4FeaturesKHR m_maintenance4Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_FEATURES_KHR };
		VkPhysicalDeviceShaderIntegerDotProductFeaturesKHR m_shaderIntegerDotProductFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_FEATURES_KHR };
		VkPhysicalDeviceShaderTerminateInvocationFeaturesKHR m_shaderTerminateInvocationFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_TERMINATE_INVOCATION_FEATURES_KHR };
		VkPhysicalDeviceSynchronization2FeaturesKHR m_synchronization2Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR };
		VkPhysicalDeviceZeroInitializeWorkgroupMemoryFeaturesKHR m_zeroInitializeWorkgroupMemoryFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ZERO_INITIALIZE_WORKGROUP_MEMORY_FEATURES_KHR };
		VkPhysicalDevice4444FormatsFeaturesEXT m_4444FormatsFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_4444_FORMATS_FEATURES_EXT };
		VkPhysicalDeviceExtendedDynamicStateFeaturesEXT m_extendedDynamicStateFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT };
		VkPhysicalDeviceExtendedDynamicState2FeaturesEXT m_extendedDynamicState2Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_2_FEATURES_EXT };
		VkPhysicalDeviceYcbcr2Plane444FormatsFeaturesEXT m_ycbcr2Plane444FormatsFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_YCBCR_2_PLANE_444_FORMATS_FEATURES_EXT };

		//Promoted to Vulkan 1.2
		VkPhysicalDevice8BitStorageFeaturesKHR m_8bitStorageFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES_KHR };
		VkPhysicalDeviceBufferDeviceAddressFeaturesKHR m_bufferDeviceAddressFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR };
		VkPhysicalDeviceSeparateDepthStencilLayoutsFeaturesKHR m_separateDepthStencilLayoutsFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SEPARATE_DEPTH_STENCIL_LAYOUTS_FEATURES_KHR };
		VkPhysicalDeviceShaderAtomicInt64FeaturesKHR m_shaderAtomicInt64Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_INT64_FEATURES_KHR };
		VkPhysicalDeviceFloat16Int8FeaturesKHR m_float16Int8Features{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT16_INT8_FEATURES_KHR };
		VkPhysicalDeviceShaderFloat16Int8FeaturesKHR m_shaderFloat16Int8Features{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR };
		VkPhysicalDeviceShaderSubgroupExtendedTypesFeaturesKHR m_shaderSubgroupExtendedTypesFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_SUBGROUP_EXTENDED_TYPES_FEATURES_KHR };
		VkPhysicalDeviceTimelineSemaphoreFeaturesKHR m_timelineSemaphoreFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES_KHR };
		VkPhysicalDeviceUniformBufferStandardLayoutFeaturesKHR m_uniformBufferStandardLayoutFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_UNIFORM_BUFFER_STANDARD_LAYOUT_FEATURES_KHR };
		VkPhysicalDeviceVulkanMemoryModelFeaturesKHR m_vulkanMemoryModelFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_MEMORY_MODEL_FEATURES_KHR };
		VkPhysicalDeviceDescriptorIndexingFeaturesEXT m_descriptorIndexingFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT };
		VkPhysicalDeviceHostQueryResetFeaturesEXT m_hostQueryResetFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES_EXT };
		VkPhysicalDeviceScalarBlockLayoutFeaturesEXT m_scalarBlockLayoutFeatures{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES_EXT };
		VkPhysicalDeviceImagelessFramebufferFeaturesKHR m_imagelessFramebufferFeatures{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_IMAGELESS_FRAMEBUFFER_FEATURES_KHR };

		VkPhysicalDeviceProperties2 m_properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2 };
		VkPhysicalDeviceSubgroupProperties m_subgroupProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES };
		VkPhysicalDeviceVulkan11Properties  m_11Properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES };
		VkPhysicalDeviceVulkan12Properties  m_12Properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES };
		VkPhysicalDeviceVulkan13Properties  m_13Properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES };
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR };
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR };
		VkPhysicalDeviceMeshShaderPropertiesEXT m_meshShaderProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT };

		//Promoted to Vulkan 1.3
		VkPhysicalDeviceSubgroupSizeControlPropertiesEXT m_subgroupSizeControlProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_PROPERTIES };
		VkPhysicalDeviceTexelBufferAlignmentPropertiesEXT m_texelBufferAlignmentProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TEXEL_BUFFER_ALIGNMENT_PROPERTIES };
		VkPhysicalDeviceInlineUniformBlockPropertiesEXT m_inlineUniformBlockProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_INLINE_UNIFORM_BLOCK_PROPERTIES };
		VkPhysicalDeviceMaintenance4PropertiesKHR m_maintenance4Properties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MAINTENANCE_4_PROPERTIES };
		VkPhysicalDeviceShaderIntegerDotProductPropertiesKHR m_shaderIntegerDotProductProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_INTEGER_DOT_PRODUCT_PROPERTIES };

		//Promoted to Vulkan 1.2
		VkPhysicalDeviceDriverPropertiesKHR m_driverProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES_KHR };
		VkPhysicalDeviceFloatControlsPropertiesKHR m_floatControlsProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FLOAT_CONTROLS_PROPERTIES_KHR };
		VkPhysicalDeviceTimelineSemaphorePropertiesKHR m_timelineSemaphoreProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_PROPERTIES_KHR };
		VkPhysicalDeviceDescriptorIndexingPropertiesEXT m_descriptorIndexingProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_PROPERTIES_EXT };
		VkPhysicalDeviceSamplerFilterMinmaxPropertiesEXT m_samplerFilterMinmaxProperties{.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SAMPLER_FILTER_MINMAX_PROPERTIES_EXT };

	};
}