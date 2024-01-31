module;

#include <cassert>
#include <string_view>

#include "volk.h"

module NYANVulkanWrapper;
import NYANLog;

using namespace nyan::vulkan::wrapper;


std::vector<const char*> PhysicalDevice::Extensions::generate_extension_list(uint32_t apiVersion) const noexcept
{
	std::vector<const char*> extensionList;

	if (swapchain)
		extensionList.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	if (fullscreenExclusive)
		extensionList.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
	if (debugUtils)
		extensionList.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	if (debugMarker)
		extensionList.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	if (accelerationStructure)
		extensionList.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
	if (rayTracingPipeline)
		extensionList.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
	if (rayQuery)
		extensionList.push_back(VK_KHR_RAY_QUERY_EXTENSION_NAME);
	if (pipelineLibrary)
		extensionList.push_back(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME);
	if (deferredHostOperations)
		extensionList.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
	if (performanceQuery)
		extensionList.push_back(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME);
	if (vertexInputDynamicState)
		extensionList.push_back(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME);
	if (meshShader)
		extensionList.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
	if (atomicFloats)
		extensionList.push_back(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME);
	if (presentId)
		extensionList.push_back(VK_KHR_PRESENT_ID_EXTENSION_NAME);
	if (presentWait)
		extensionList.push_back(VK_KHR_PRESENT_WAIT_EXTENSION_NAME);

	if (apiVersion < VK_API_VERSION_1_3) {
		if (copyCommands)
			extensionList.push_back(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME);
		if (dynamicRendering)
			extensionList.push_back(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
		if (formatFeatureFlags2)
			extensionList.push_back(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME);
		if (maintenance4)
			extensionList.push_back(VK_KHR_MAINTENANCE_4_EXTENSION_NAME);
		if (shaderIntegerDotProduct)
			extensionList.push_back(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME);
		if (shaderNonSemanticInfo)
			extensionList.push_back(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
		if (shaderTerminateInvocation)
			extensionList.push_back(VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME);
		if (synchronization2)
			extensionList.push_back(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME);
		if (zeroInitializeWorkgroupMemory)
			extensionList.push_back(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME);
		if (formats4444)
			extensionList.push_back(VK_EXT_4444_FORMATS_EXTENSION_NAME);
		if (extendedDynamicState)
			extensionList.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
		if (extendedDynamicState2)
			extensionList.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME);
		if (imageRobustness)
			extensionList.push_back(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME);
		if (inlineUniformBlock)
			extensionList.push_back(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME);
		if (pipelineCreationCacheControl)
			extensionList.push_back(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME);
		if (pipelineCreationFeedback)
			extensionList.push_back(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME);
		if (privateData)
			extensionList.push_back(VK_EXT_PRIVATE_DATA_EXTENSION_NAME);
		if (shaderDemoteToHelperInvocation)
			extensionList.push_back(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME);
		if (subgroupSizeControl)
			extensionList.push_back(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME);
		if (texelBufferAlignment)
			extensionList.push_back(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME);
		if (textureCompressionAstcHdr)
			extensionList.push_back(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME);
		if (toolingInfo)
			extensionList.push_back(VK_EXT_TOOLING_INFO_EXTENSION_NAME);
		if (ycbcr2plane444Formats)
			extensionList.push_back(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME);
	}

	if (apiVersion < VK_API_VERSION_1_2) {
		if (bit8Storage)
			extensionList.push_back(VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
		if (bufferDeviceAddress)
			extensionList.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
		if (createRenderpass2)
			extensionList.push_back(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
		if (depthStencilResolve)
			extensionList.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
		if (drawIndirectCount)
			extensionList.push_back(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME);
		if (driverProperties)
			extensionList.push_back(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME);
		if (imageFormatList)
			extensionList.push_back(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
		if (imagelessFramebuffer)
			extensionList.push_back(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME);
		if (samplerMirrorClampToEdge)
			extensionList.push_back(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME);
		if (separateDepthStencilLayouts)
			extensionList.push_back(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME);
		if (shaderAtomicInt64)
			extensionList.push_back(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME);
		if (shaderFloat16Int8)
			extensionList.push_back(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
		if (shaderFloatControls)
			extensionList.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
		if (shaderSubgroupExtendedTypes)
			extensionList.push_back(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME);
		if (spirv14)
			extensionList.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME);
		if (timelineSemaphore)
			extensionList.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
		if (uniformBufferStandardLayout)
			extensionList.push_back(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME);
		if (vulkanMemoryModel)
			extensionList.push_back(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
		if (descriptorIndexing)
			extensionList.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
		if (hostQueryReset)
			extensionList.push_back(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME);
		if (samplerFilterMinmax)
			extensionList.push_back(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME);
		if (scalarBlockLayout)
			extensionList.push_back(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME);
		if (separateStencilUsage)
			extensionList.push_back(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME);
		if (shaderViewportIndexLayer)
			extensionList.push_back(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME);
	}

	return extensionList;
}

PhysicalDevice::Extensions PhysicalDevice::Extensions::generate_dependent_extensions(uint32_t apiVersion) const noexcept
{
	Extensions dependencies = *this;
	if (dependencies.accelerationStructure) {
		if (apiVersion < VK_API_VERSION_1_2) {
			dependencies.bufferDeviceAddress = 1;
			dependencies.descriptorIndexing = 1;
		}
		dependencies.deferredHostOperations = 1;
	}

	if (dependencies.presentWait)
		dependencies.presentId = 1;

	if (dependencies.presentId)
		dependencies.swapchain = 1;

	if (dependencies.fullscreenExclusive)
		dependencies.swapchain = 1;

	if (dependencies.dynamicRendering)
		dependencies.depthStencilResolve = 1;

	if (dependencies.depthStencilResolve)
		dependencies.createRenderpass2 = 1;

	if (dependencies.imagelessFramebuffer)
		dependencies.imageFormatList = 1;

	if (dependencies.separateDepthStencilLayouts)
		dependencies.createRenderpass2 = 1;

	if (dependencies.spirv14)
		dependencies.shaderFloatControls = 1;

	return dependencies;
}

PhysicalDevice::Extensions PhysicalDevice::Extensions::generate_extension(std::string_view extensionName) noexcept
{
	if (extensionName.compare(VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		return Extensions{ .swapchain = 1 };
	if (extensionName.compare(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0)
		return Extensions { .fullscreenExclusive = 1 };
	if (extensionName.compare(VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0)
		return Extensions { .debugUtils = 1 };
	if (extensionName.compare(VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
		return Extensions { .debugMarker = 1 };
	if (extensionName.compare(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0)
		return Extensions { .accelerationStructure = 1 };
	if (extensionName.compare(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0)
		return Extensions { .rayTracingPipeline = 1 };
	if (extensionName.compare(VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0)
		return Extensions { .rayQuery = 1 };
	if (extensionName.compare(VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME) == 0)
		return Extensions { .pipelineLibrary = 1 };
	if (extensionName.compare(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0)
		return Extensions { .deferredHostOperations = 1 };
	if (extensionName.compare(VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0)
		return Extensions { .performanceQuery = 1 };
	if (extensionName.compare(VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME) == 0)
		return Extensions { .vertexInputDynamicState = 1 };
	if (extensionName.compare(VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0)
		return Extensions { .meshShader = 1 };
	if (extensionName.compare(VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME) == 0)
		return Extensions { .atomicFloats = 1 };
	if (extensionName.compare(VK_KHR_PRESENT_ID_EXTENSION_NAME) == 0)
		return Extensions { .presentId = 1 };
	if (extensionName.compare(VK_KHR_PRESENT_WAIT_EXTENSION_NAME) == 0)
		return Extensions { .presentWait = 1 };

	if (extensionName.compare(VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME) == 0)
		return Extensions { .copyCommands = 1 };
	if (extensionName.compare(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0)
		return Extensions { .dynamicRendering = 1 };
	if (extensionName.compare(VK_KHR_FORMAT_FEATURE_FLAGS_2_EXTENSION_NAME) == 0)
		return Extensions { .formatFeatureFlags2 = 1 };
	if (extensionName.compare(VK_KHR_MAINTENANCE_4_EXTENSION_NAME) == 0)
		return Extensions { .maintenance4 = 1 };
	if (extensionName.compare(VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME) == 0)
		return Extensions { .shaderIntegerDotProduct = 1 };
	if (extensionName.compare(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME) == 0)
		return Extensions { .shaderNonSemanticInfo = 1 };
	if (extensionName.compare(VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME) == 0)
		return Extensions { .shaderTerminateInvocation = 1 };
	if (extensionName.compare(VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0)
		return Extensions { .synchronization2 = 1 };
	if (extensionName.compare(VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME) == 0)
		return Extensions { .zeroInitializeWorkgroupMemory = 1 };
	if (extensionName.compare(VK_EXT_4444_FORMATS_EXTENSION_NAME) == 0)
		return Extensions { .formats4444 = 1 };
	if (extensionName.compare(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0)
		return Extensions { .extendedDynamicState = 1 };
	if (extensionName.compare(VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0)
		return Extensions { .extendedDynamicState2 = 1 };
	if (extensionName.compare(VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME) == 0)
		return Extensions { .imageRobustness = 1 };
	if (extensionName.compare(VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME) == 0)
		return Extensions { .inlineUniformBlock = 1 };
	if (extensionName.compare(VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0)
		return Extensions { .pipelineCreationCacheControl = 1 };
	if (extensionName.compare(VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME) == 0)
		return Extensions { .pipelineCreationFeedback = 1 };
	if (extensionName.compare(VK_EXT_PRIVATE_DATA_EXTENSION_NAME) == 0)
		return Extensions { .privateData = 1 };
	if (extensionName.compare(VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME) == 0)
		return Extensions { .shaderDemoteToHelperInvocation = 1 };
	if (extensionName.compare(VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME) == 0)
		return Extensions { .subgroupSizeControl = 1 };
	if (extensionName.compare(VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME) == 0)
		return Extensions { .texelBufferAlignment = 1 };
	if (extensionName.compare(VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME) == 0)
		return Extensions { .textureCompressionAstcHdr = 1 };
	if (extensionName.compare(VK_EXT_TOOLING_INFO_EXTENSION_NAME) == 0)
		return Extensions { .toolingInfo = 1 };
	if (extensionName.compare(VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME) == 0)
		return Extensions { .ycbcr2plane444Formats = 1 };

	if (extensionName.compare(VK_KHR_8BIT_STORAGE_EXTENSION_NAME) == 0)
		return Extensions { .bit8Storage = 1 };
	if (extensionName.compare(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0)
		return Extensions { .bufferDeviceAddress = 1 };
	if (extensionName.compare(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0)
		return Extensions { .createRenderpass2 = 1 };
	if (extensionName.compare(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0)
		return Extensions { .depthStencilResolve = 1 };
	if (extensionName.compare(VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0)
		return Extensions { .drawIndirectCount = 1 };
	if (extensionName.compare(VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) == 0)
		return Extensions { .driverProperties = 1 };
	if (extensionName.compare(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME) == 0)
		return Extensions { .imageFormatList = 1 };
	if (extensionName.compare(VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME) == 0)
		return Extensions { .imagelessFramebuffer = 1 };
	if (extensionName.compare(VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) == 0)
		return Extensions { .samplerMirrorClampToEdge = 1 };
	if (extensionName.compare(VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME) == 0)
		return Extensions { .separateDepthStencilLayouts = 1 };
	if (extensionName.compare(VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME) == 0)
		return Extensions { .shaderAtomicInt64 = 1 };
	if (extensionName.compare(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) == 0)
		return Extensions { .shaderFloat16Int8 = 1 };
	if (extensionName.compare(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME) == 0)
		return Extensions { .shaderFloatControls = 1 };
	if (extensionName.compare(VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME) == 0)
		return Extensions { .shaderSubgroupExtendedTypes = 1 };
	if (extensionName.compare(VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0)
		return Extensions { .spirv14 = 1 };
	if (extensionName.compare(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0)
		return Extensions { .timelineSemaphore = 1 };
	if (extensionName.compare(VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME) == 0)
		return Extensions { .uniformBufferStandardLayout = 1 };
	if (extensionName.compare(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME) == 0)
		return Extensions { .vulkanMemoryModel = 1 };
	if (extensionName.compare(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0)
		return Extensions { .descriptorIndexing = 1 };
	if (extensionName.compare(VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0)
		return Extensions { .hostQueryReset = 1 };
	if (extensionName.compare(VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0)
		return Extensions { .samplerFilterMinmax = 1 };
	if (extensionName.compare(VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME) == 0)
		return Extensions { .scalarBlockLayout = 1 };
	if (extensionName.compare(VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME) == 0)
		return Extensions { .separateStencilUsage = 1 };
	if (extensionName.compare(VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0)
		return Extensions { .shaderViewportIndexLayer = 1 };

	return Extensions{ };
}

const VkPhysicalDeviceFeatures& PhysicalDevice::get_used_features() const noexcept
{
	return m_features.features;
}

const VkPhysicalDeviceVulkan11Features& PhysicalDevice::get_vulkan11_features() const noexcept
{
	return m_11Features;
}

const VkPhysicalDeviceVulkan12Features& PhysicalDevice::get_vulkan12_features() const noexcept
{
	return m_12Features;
}

const VkPhysicalDeviceVulkan13Features& PhysicalDevice::get_vulkan13_features() const noexcept
{
	return m_13Features;
}

const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& PhysicalDevice::get_atomic_float_features() const noexcept
{
	return m_atomicFloatFeatures;
}

const VkPhysicalDeviceAccelerationStructureFeaturesKHR& PhysicalDevice::get_acceleration_structure_features() const noexcept
{
	return m_accelerationStructureFeatures;
}

const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& PhysicalDevice::get_ray_tracing_pipeline_features() const noexcept
{
	return m_rayTracingPipelineFeatures;
}

const VkPhysicalDeviceRayQueryFeaturesKHR& PhysicalDevice::get_ray_query_features() const noexcept
{
	return m_rayQueryFeatures;
}

const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& PhysicalDevice::get_vertex_input_dynamic_state_features() const noexcept
{
	return m_vertexInputDynamicStateFeatures;
}

const VkPhysicalDeviceMeshShaderFeaturesEXT& PhysicalDevice::get_mesh_shader_features() const noexcept
{
	return m_meshShaderFeatures;
}

const VkPhysicalDeviceProperties& PhysicalDevice::get_properties() const noexcept
{
	return m_properties.properties;
}

const VkPhysicalDeviceSubgroupProperties& PhysicalDevice::get_subgroup_properties() const noexcept
{
	return m_subgroupProperties;
}

const VkPhysicalDeviceVulkan11Properties& PhysicalDevice::get_vulkan11_properties() const noexcept
{
	return m_11Properties;
}

const VkPhysicalDeviceVulkan12Properties& PhysicalDevice::get_vulkan12_properties() const noexcept
{
	return m_12Properties;
}

const VkPhysicalDeviceVulkan13Properties& PhysicalDevice::get_vulkan13_properties() const noexcept
{
	return m_13Properties;
}

const VkPhysicalDeviceAccelerationStructurePropertiesKHR& PhysicalDevice::get_acceleration_structure_properties() const noexcept
{
	return m_accelerationStructureProperties;
}

const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& PhysicalDevice::get_ray_tracing_pipeline_properties() const noexcept
{
	return m_rayTracingPipelineProperties;
}

const VkPhysicalDeviceMeshShaderPropertiesEXT& PhysicalDevice::get_mesh_shader_properties() const noexcept
{
	return m_meshShaderProperties;
}

const PhysicalDevice::Extensions& PhysicalDevice::get_available_extensions() const noexcept
{
	return m_availableExtensions;
}

std::expected<PhysicalDevice, PhysicalDeviceCreationError> PhysicalDevice::create(VkPhysicalDevice handle)
{
	uint32_t extensionCount;
	if (auto result = vkEnumerateDeviceExtensionProperties(handle, nullptr, &extensionCount, nullptr); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ PhysicalDeviceCreationError::Type::OutOfMemoryError };
		else if (result == VK_ERROR_LAYER_NOT_PRESENT)
			return std::unexpected{ PhysicalDeviceCreationError::Type::LayerNotPresent };
		else
			return std::unexpected{ PhysicalDeviceCreationError::Type::UnknownError };
	}
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	if (auto result = vkEnumerateDeviceExtensionProperties(handle, nullptr, &extensionCount, availableExtensions.data()); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ PhysicalDeviceCreationError::Type::OutOfMemoryError };
		else if (result == VK_ERROR_LAYER_NOT_PRESENT)
			return std::unexpected{ PhysicalDeviceCreationError::Type::LayerNotPresent };
		else
			return std::unexpected{ PhysicalDeviceCreationError::Type::UnknownError };
	}
	Extensions availableExtension;
	for(const auto& extensionProperty : availableExtensions)
	{
		availableExtension = availableExtension & Extensions::generate_extension(extensionProperty.extensionName);
	}
	

	return PhysicalDevice{ handle, availableExtension };
}

const VkPhysicalDeviceFeatures2& PhysicalDevice::build_feature_chain(const Extensions& extensions) noexcept
{

	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_1)
	{
		m_11Features.pNext = m_features.pNext;
		m_features.pNext = &m_11Features;
	}
	else
	{
		assert(false);
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_2)
	{
		m_12Features.pNext = m_features.pNext;
		m_features.pNext = &m_12Features;
	}
	else
	{
		if (extensions.bit8Storage) {
			m_8bitStorageFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_8bitStorageFeatures;
		}
		if (extensions.bufferDeviceAddress) {
			m_bufferDeviceAddressFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_bufferDeviceAddressFeatures;
		}
		if (extensions.separateDepthStencilLayouts) {
			m_separateDepthStencilLayoutsFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_separateDepthStencilLayoutsFeatures;
		}
		if (extensions.shaderAtomicInt64) {
			m_shaderAtomicInt64Features.pNext = m_features.pNext;
			m_features.pNext = &m_shaderAtomicInt64Features;
		}
		if (extensions.shaderFloat16Int8) {
			m_shaderFloat16Int8Features.pNext = m_features.pNext;
			m_features.pNext = &m_shaderFloat16Int8Features;
		}
		if (extensions.shaderSubgroupExtendedTypes) {
			m_shaderSubgroupExtendedTypesFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderSubgroupExtendedTypesFeatures;
		}
		if (extensions.timelineSemaphore) {
			m_timelineSemaphoreFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_timelineSemaphoreFeatures;
		}
		if (extensions.uniformBufferStandardLayout) {
			m_uniformBufferStandardLayoutFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_uniformBufferStandardLayoutFeatures;
		}
		if (extensions.vulkanMemoryModel) {
			m_vulkanMemoryModelFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_vulkanMemoryModelFeatures;
		}
		if (extensions.descriptorIndexing) {
			m_descriptorIndexingFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_descriptorIndexingFeatures;
		}
		if (extensions.hostQueryReset) {
			m_hostQueryResetFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_hostQueryResetFeatures;
		}
		if (extensions.scalarBlockLayout) {
			m_scalarBlockLayoutFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_scalarBlockLayoutFeatures;
		}
		if (extensions.imagelessFramebuffer) {
			m_imagelessFramebufferFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_imagelessFramebufferFeatures;
		}
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_3)
	{
		m_13Features.pNext = m_features.pNext;
		m_features.pNext = &m_13Features;
	}
	else
	{
		if (extensions.pipelineCreationCacheControl) {
			m_pipelineCreationCacheControlFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_pipelineCreationCacheControlFeatures;
		}
		if (extensions.privateData) {
			m_privateDataFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_privateDataFeatures;
		}
		if (extensions.shaderDemoteToHelperInvocation) {
			m_shaderDemoteToHelperInvocationFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderDemoteToHelperInvocationFeatures;
		}
		if (extensions.subgroupSizeControl) {
			m_subgroupSizeControlFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_subgroupSizeControlFeatures;
		}
		if (extensions.texelBufferAlignment) {
			m_texelBufferAlignmentFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_texelBufferAlignmentFeatures;
		}
		if (extensions.textureCompressionAstcHdr) {
			m_textureCompressionAstchdrFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_textureCompressionAstchdrFeatures;
		}
		if (extensions.imageRobustness) {
			m_imageRobustnessFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_imageRobustnessFeatures;
		}
		if (extensions.inlineUniformBlock) {
			m_inlineUniformBlockFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_inlineUniformBlockFeatures;
		}
		if (extensions.dynamicRendering) {
			m_dynamicRenderingFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_dynamicRenderingFeatures;
		}
		if (extensions.maintenance4) {
			m_maintenance4Features.pNext = m_features.pNext;
			m_features.pNext = &m_maintenance4Features;
		}
		if (extensions.shaderIntegerDotProduct) {
			m_shaderIntegerDotProductFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderIntegerDotProductFeatures;
		}
		if (extensions.shaderTerminateInvocation) {
			m_shaderTerminateInvocationFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderTerminateInvocationFeatures;
		}
		if (extensions.synchronization2) {
			m_synchronization2Features.pNext = m_features.pNext;
			m_features.pNext = &m_synchronization2Features;
		}
		if (extensions.zeroInitializeWorkgroupMemory) {
			m_zeroInitializeWorkgroupMemoryFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_zeroInitializeWorkgroupMemoryFeatures;
		}
		if (extensions.formats4444) {
			m_4444FormatsFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_4444FormatsFeatures;
		}
		if (extensions.extendedDynamicState) {
			m_extendedDynamicStateFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_extendedDynamicStateFeatures;
		}
		if (extensions.extendedDynamicState2) {
			m_extendedDynamicState2Features.pNext = m_features.pNext;
			m_features.pNext = &m_extendedDynamicState2Features;
		}
		if (extensions.ycbcr2plane444Formats) {
			m_ycbcr2Plane444FormatsFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_ycbcr2Plane444FormatsFeatures;
		}
	}

	if (extensions.accelerationStructure)
	{
		m_accelerationStructureFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_accelerationStructureFeatures;
	}
	if (extensions.rayTracingPipeline)
	{
		m_rayTracingPipelineFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_rayTracingPipelineFeatures;
	}
	if (extensions.rayQuery)
	{
		m_rayQueryFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_rayQueryFeatures;
	}
	if (extensions.vertexInputDynamicState)
	{
		m_vertexInputDynamicStateFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_vertexInputDynamicStateFeatures;
	}
	if (extensions.meshShader)
	{
		m_meshShaderFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_meshShaderFeatures;
	}
	if (extensions.atomicFloats)
	{
		m_atomicFloatFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_atomicFloatFeatures;
	}
	if (extensions.presentId)
	{
		m_presentIdFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_presentIdFeatures;
	}
	if (extensions.presentWait)
	{
		m_presentWaitFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_presentWaitFeatures;
	}
	return m_features;
}

VkPhysicalDevice PhysicalDevice::get_handle() const noexcept
{
	return m_handle;
}

uint32_t nyan::vulkan::wrapper::PhysicalDevice::get_queue_family_index(Queue::Type queueType) const noexcept
{
	assert(queueType != Queue::Type::Size);
	return m_queueFamilyIndices[static_cast<size_t>(queueType)];
}

PhysicalDevice::Type PhysicalDevice::get_type() const noexcept
{
	return m_type;
}

uint64_t PhysicalDevice::get_id() const noexcept
{
	return (static_cast<uint64_t>(m_properties.properties.vendorID) << 32ull) | static_cast<uint64_t>(m_properties.properties.deviceID);
}

PhysicalDevice::PhysicalDevice(VkPhysicalDevice handle, const Extensions& availableExtensions) noexcept :
	m_handle(handle),
	m_availableExtensions(availableExtensions)
{
	init_type();
	init_queues();
	init_features();
	init_properties();
}

void PhysicalDevice::init_type() noexcept
{
	const auto& properties = m_properties.properties;
	if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
		m_type = Type::Integrated;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		m_type = Type::Discrete;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
		m_type = Type::Virtual;
	else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
		m_type = Type::CPU;
	else
		m_type = Type::Other;
}

void PhysicalDevice::init_queues() noexcept
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &numQueueFamilies, nullptr);
	m_queueFamilyProperties.resize(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(m_handle, &numQueueFamilies, m_queueFamilyProperties.data());

	for (uint32_t i = 0; i < m_queueFamilyProperties.size(); i++) {
		const auto& queue = m_queueFamilyProperties[i];
		if ((queue.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_VIDEO_ENCODE_BIT_KHR | VK_QUEUE_VIDEO_DECODE_BIT_KHR)) == VK_QUEUE_TRANSFER_BIT) {
			if (m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Transfer)] == ~0u)
				m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Transfer)] = i;
		}
		else if ((queue.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == VK_QUEUE_COMPUTE_BIT) {
			m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Compute)] = i;
		}
		else if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
			m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Graphics)] = i;
		}
		else if ((queue.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR) == VK_QUEUE_VIDEO_ENCODE_BIT_KHR) {
			m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Encode)] = i;
		}
		else if ((queue.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR) == VK_QUEUE_VIDEO_DECODE_BIT_KHR) {
			m_queueFamilyIndices[static_cast<size_t>(Queue::Type::Decode)] = i;
		}
	}
}

void PhysicalDevice::init_features() noexcept
{
	vkGetPhysicalDeviceProperties(m_handle, &m_properties.properties);
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		vkGetPhysicalDeviceFeatures(m_handle, &m_features.features);
		return;
	}
	m_features.pNext = &m_11Features;

	m_11Features.pNext = &m_12Features;

	m_12Features.pNext = &m_13Features;

	m_13Features.pNext = &m_atomicFloatFeatures;

	m_atomicFloatFeatures.pNext = &m_accelerationStructureFeatures;

	m_accelerationStructureFeatures.pNext = &m_rayTracingPipelineFeatures;

	m_rayTracingPipelineFeatures.pNext = &m_rayQueryFeatures;

	m_rayQueryFeatures.pNext = &m_vertexInputDynamicStateFeatures;

	m_vertexInputDynamicStateFeatures.pNext = &m_meshShaderFeatures;

	m_meshShaderFeatures.pNext = &m_presentIdFeatures;

	m_presentIdFeatures.pNext = &m_presentWaitFeatures;

	m_presentWaitFeatures.pNext = nullptr;


	if (m_properties.properties.apiVersion < VK_API_VERSION_1_3)
	{
		m_presentWaitFeatures.pNext = &m_pipelineCreationCacheControlFeatures;
		m_pipelineCreationCacheControlFeatures.pNext = &m_privateDataFeatures;
		m_privateDataFeatures.pNext = &m_shaderDemoteToHelperInvocationFeatures;
		m_shaderDemoteToHelperInvocationFeatures.pNext = &m_subgroupSizeControlFeatures;
		m_subgroupSizeControlFeatures.pNext = &m_texelBufferAlignmentFeatures;
		m_textureCompressionAstchdrFeatures.pNext = &m_imageRobustnessFeatures;
		m_imageRobustnessFeatures.pNext = &m_inlineUniformBlockFeatures;
		m_inlineUniformBlockFeatures.pNext = &m_dynamicRenderingFeatures;
		m_dynamicRenderingFeatures.pNext = &m_maintenance4Features;
		m_maintenance4Features.pNext = &m_shaderIntegerDotProductFeatures;
		m_shaderIntegerDotProductFeatures.pNext = &m_shaderTerminateInvocationFeatures;
		m_shaderTerminateInvocationFeatures.pNext = &m_synchronization2Features;
		m_synchronization2Features.pNext = &m_zeroInitializeWorkgroupMemoryFeatures;
		m_zeroInitializeWorkgroupMemoryFeatures.pNext = &m_4444FormatsFeatures;
		m_4444FormatsFeatures.pNext = &m_extendedDynamicStateFeatures;
		m_extendedDynamicStateFeatures.pNext = &m_extendedDynamicState2Features;
		m_extendedDynamicState2Features.pNext = &m_ycbcr2Plane444FormatsFeatures;
		m_ycbcr2Plane444FormatsFeatures.pNext = nullptr;
	}


	if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
	{
		m_extendedDynamicState2Features.pNext = &m_8bitStorageFeatures;
		m_8bitStorageFeatures.pNext = &m_bufferDeviceAddressFeatures;
		m_bufferDeviceAddressFeatures.pNext = &m_separateDepthStencilLayoutsFeatures;
		m_separateDepthStencilLayoutsFeatures.pNext = &m_shaderAtomicInt64Features;
		m_shaderAtomicInt64Features.pNext = &m_shaderFloat16Int8Features;
		m_shaderFloat16Int8Features.pNext = &m_shaderSubgroupExtendedTypesFeatures;
		m_shaderSubgroupExtendedTypesFeatures.pNext = &m_timelineSemaphoreFeatures;
		m_timelineSemaphoreFeatures.pNext = &m_uniformBufferStandardLayoutFeatures;
		m_uniformBufferStandardLayoutFeatures.pNext = &m_vulkanMemoryModelFeatures;
		m_vulkanMemoryModelFeatures.pNext = &m_descriptorIndexingFeatures;
		m_descriptorIndexingFeatures.pNext = &m_hostQueryResetFeatures;
		m_hostQueryResetFeatures.pNext = &m_scalarBlockLayoutFeatures;
		m_scalarBlockLayoutFeatures.pNext = &m_imagelessFramebufferFeatures;
		m_imagelessFramebufferFeatures.pNext = nullptr;
	}

	vkGetPhysicalDeviceFeatures2(m_handle, &m_features);

	m_features.pNext = nullptr;


	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_3)
	{
		m_availableExtensions.copyCommands = 1;
		m_availableExtensions.dynamicRendering = m_13Features.dynamicRendering;
		m_availableExtensions.formatFeatureFlags2 = 1;
		m_availableExtensions.maintenance4 = m_13Features.maintenance4;
		m_availableExtensions.shaderIntegerDotProduct = m_13Features.shaderIntegerDotProduct;
		m_availableExtensions.shaderNonSemanticInfo = 1;
		m_availableExtensions.shaderTerminateInvocation = m_13Features.shaderTerminateInvocation;
		m_availableExtensions.synchronization2 = m_13Features.synchronization2;
		m_availableExtensions.zeroInitializeWorkgroupMemory = m_13Features.shaderZeroInitializeWorkgroupMemory;
		m_availableExtensions.formats4444 = 1;
		m_availableExtensions.extendedDynamicState = 1;
		m_availableExtensions.extendedDynamicState2 = 1;
		m_availableExtensions.imageRobustness = m_13Features.robustImageAccess;
		m_availableExtensions.inlineUniformBlock = m_13Features.inlineUniformBlock;
		m_availableExtensions.pipelineCreationCacheControl = m_13Features.pipelineCreationCacheControl;
		m_availableExtensions.pipelineCreationFeedback = 1;
		m_availableExtensions.privateData = m_13Features.privateData;
		m_availableExtensions.shaderDemoteToHelperInvocation = m_13Features.shaderDemoteToHelperInvocation;
		m_availableExtensions.subgroupSizeControl = m_13Features.subgroupSizeControl;
		m_availableExtensions.texelBufferAlignment = 1;
		m_availableExtensions.textureCompressionAstcHdr = m_13Features.textureCompressionASTC_HDR;
		m_availableExtensions.toolingInfo = 1;
		m_availableExtensions.ycbcr2plane444Formats = 1;
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_2)
	{
		m_availableExtensions.bit8Storage = m_12Features.storageBuffer8BitAccess && m_12Features.uniformAndStorageBuffer8BitAccess && m_12Features.storagePushConstant8;
		m_availableExtensions.bufferDeviceAddress = m_12Features.bufferDeviceAddress;
		m_availableExtensions.createRenderpass2 = 1;
		m_availableExtensions.depthStencilResolve = 1;
		m_availableExtensions.drawIndirectCount = m_12Features.drawIndirectCount;
		m_availableExtensions.driverProperties = 1;
		m_availableExtensions.imageFormatList = 1;
		m_availableExtensions.imagelessFramebuffer = m_12Features.imagelessFramebuffer;
		m_availableExtensions.samplerMirrorClampToEdge = m_12Features.samplerMirrorClampToEdge;
		m_availableExtensions.separateDepthStencilLayouts = m_12Features.separateDepthStencilLayouts;
		m_availableExtensions.shaderAtomicInt64 = m_12Features.shaderBufferInt64Atomics && m_12Features.shaderSharedInt64Atomics;
		m_availableExtensions.shaderFloat16Int8 = m_12Features.shaderFloat16 && m_12Features.shaderInt8;
		m_availableExtensions.shaderFloatControls = 1;
		m_availableExtensions.shaderSubgroupExtendedTypes = m_12Features.shaderSubgroupExtendedTypes;
		m_availableExtensions.spirv14 = 1;
		m_availableExtensions.timelineSemaphore = m_12Features.timelineSemaphore;
		m_availableExtensions.uniformBufferStandardLayout = m_12Features.uniformBufferStandardLayout;
		m_availableExtensions.vulkanMemoryModel = m_12Features.vulkanMemoryModel;
		m_availableExtensions.descriptorIndexing = m_12Features.descriptorIndexing;
		m_availableExtensions.hostQueryReset = m_12Features.hostQueryReset;
		m_availableExtensions.samplerFilterMinmax = m_12Features.samplerFilterMinmax;
		m_availableExtensions.scalarBlockLayout = m_12Features.scalarBlockLayout;
		m_availableExtensions.separateStencilUsage = 1;
		m_availableExtensions.shaderViewportIndexLayer = m_12Features.shaderOutputViewportIndex;
	}
}

void PhysicalDevice::init_properties() noexcept
{
	vkGetPhysicalDeviceProperties(m_handle, &m_properties.properties);
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		return;
	}
	m_properties.pNext = &m_subgroupProperties;

	m_subgroupProperties.pNext = &m_11Properties;

	m_11Properties.pNext = &m_12Properties;
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
		m_11Properties.pNext = &m_accelerationStructureProperties;

	m_12Properties.pNext = &m_13Properties;
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_3)
		m_12Properties.pNext = &m_accelerationStructureProperties;

	m_13Properties.pNext = &m_accelerationStructureProperties;

	m_accelerationStructureProperties.pNext = &m_rayTracingPipelineProperties;

	m_rayTracingPipelineProperties.pNext = &m_meshShaderProperties;

	m_meshShaderProperties.pNext = nullptr;

	if (m_properties.properties.apiVersion < VK_API_VERSION_1_3)
	{
		m_meshShaderProperties.pNext = &m_subgroupSizeControlProperties;
		m_subgroupSizeControlProperties.pNext = &m_texelBufferAlignmentProperties;
		m_texelBufferAlignmentProperties.pNext = &m_inlineUniformBlockProperties;
		m_inlineUniformBlockProperties.pNext = &m_maintenance4Properties;
		m_maintenance4Properties.pNext = &m_shaderIntegerDotProductProperties;
		m_shaderIntegerDotProductProperties.pNext = nullptr;
	}

	if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
	{
		m_shaderIntegerDotProductProperties.pNext = &m_driverProperties;
		m_driverProperties.pNext = &m_floatControlsProperties;
		m_floatControlsProperties.pNext = &m_timelineSemaphoreProperties;
		m_timelineSemaphoreProperties.pNext = &m_descriptorIndexingProperties;
		m_descriptorIndexingProperties.pNext = &m_samplerFilterMinmaxProperties;
		m_samplerFilterMinmaxProperties.pNext = nullptr;
	}

	vkGetPhysicalDeviceProperties2(m_handle, &m_properties);
}
