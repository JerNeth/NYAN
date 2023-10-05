#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/LogicalDevice.h"
#include "Utility/Exceptions.h"


vulkan::PhysicalDevice::PhysicalDevice(VkPhysicalDevice device) :
	m_vkHandle(device)
{
	init_extensions();
	init_queues();
	init_features();
	init_properties();
	//std::vector<std::pair<VkFormat, VkFormatProperties2>> formatProperties;
	//std::vector<std::pair<VkFormat, VkFormatProperties2>> formatProperties2;
	//std::vector<std::pair<VkFormat, VkFormatProperties2>> formatProperties3;
	//for (auto format = VK_FORMAT_UNDEFINED; format != VK_FORMAT_ASTC_12x12_SRGB_BLOCK; format = static_cast<VkFormat>(static_cast<size_t>(format) + 1)) {
	//	VkFormatProperties2 pFormatProperties{
	//		.sType = VK_STRUCTURE_TYPE_FORMAT_PROPERTIES_2
	//	};
	//	vkGetPhysicalDeviceFormatProperties2(device,format,&pFormatProperties);
	//	if (pFormatProperties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
	//		formatProperties.push_back({ format, pFormatProperties });
	//	if (pFormatProperties.formatProperties.bufferFeatures & VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)
	//		formatProperties2.push_back({ format, pFormatProperties });
	//	if ((pFormatProperties.formatProperties.bufferFeatures & (VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT | VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR)) == 
	//		(VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT | VK_FORMAT_FEATURE_ACCELERATION_STRUCTURE_VERTEX_BUFFER_BIT_KHR))
	//		formatProperties3.push_back({ format, pFormatProperties });
	//}
}
vulkan::PhysicalDevice::operator VkPhysicalDevice() const noexcept
{
	return m_vkHandle;
}

const VkPhysicalDevice& vulkan::PhysicalDevice::get_handle() const noexcept
{
	return m_vkHandle;
}

bool vulkan::PhysicalDevice::use_extension(const char* extension) noexcept
{
	for (const auto& ext : m_availableExtensions) {
		if (strcmp(ext.extensionName, extension) == 0) {
			if (strcmp(ext.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {

				if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
					break;

				bool dependencies = true;
				if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
					dependencies &= use_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
				dependencies &= m_extensions.bufferDeviceAddress;

				if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
					dependencies &= use_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);
				dependencies &= m_extensions.descriptorIndexing;

				dependencies &= use_extension(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
				if (!dependencies)
					break;
				m_extensions.acceleration_structure = m_accelerationStructureFeatures.accelerationStructure;
			}
			else if (strcmp(ext.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
				m_extensions.ray_tracing_pipeline = m_rayTracingPipelineFeatures.rayTracingPipeline;
			}
			else if (strcmp(ext.extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
				m_extensions.ray_query = m_rayQueryFeatures.rayQuery;
			}
			else if (strcmp(ext.extensionName, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
				m_extensions.vertex_input_dynamic_state = m_vertexInputDynamicStateFeatures.vertexInputDynamicState;
			}
			else if (strcmp(ext.extensionName, VK_EXT_MESH_SHADER_EXTENSION_NAME) == 0) {
				m_extensions.mesh_shader = m_meshShaderFeatures.meshShader & m_meshShaderFeatures.taskShader;
			}
			else if (strcmp(ext.extensionName, VK_EXT_SHADER_ATOMIC_FLOAT_EXTENSION_NAME) == 0) {
				m_extensions.atomic_floats = m_atomicFloatFeatures.shaderBufferFloat32Atomics & m_atomicFloatFeatures.shaderBufferFloat32AtomicAdd;
			}
			else if (strcmp(ext.extensionName, VK_KHR_PRESENT_ID_EXTENSION_NAME) == 0) {
				bool dependencies = true;
				dependencies &= use_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				if (!dependencies)
					break;
				m_extensions.present_id = m_presentIdFeatures.presentId;
			}
			else if (strcmp(ext.extensionName, VK_KHR_PRESENT_WAIT_EXTENSION_NAME) == 0) {
				bool dependencies = true;
				dependencies &= use_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				dependencies &= use_extension(VK_KHR_PRESENT_ID_EXTENSION_NAME);
				if (!dependencies)
					break;
				m_extensions.present_wait = m_presentWaitFeatures.presentWait;
			}
			else if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
				m_extensions.swapchain = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
				bool dependencies = true;
				dependencies &= use_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				if (!dependencies)
					break;
				m_extensions.fullscreen_exclusive = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
				m_extensions.debug_marker = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
				m_extensions.debug_utils = 1;
			}
			else if (strcmp(ext.extensionName, VK_KHR_PIPELINE_LIBRARY_EXTENSION_NAME) == 0) {
				m_extensions.pipeline_library = 1;
			}
			else if (strcmp(ext.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
				m_extensions.deferred_host_operations = 1;
			}
			else if (strcmp(ext.extensionName, VK_KHR_PERFORMANCE_QUERY_EXTENSION_NAME) == 0) {
				m_extensions.performance_query = 1;
			}
			if (m_properties.properties.apiVersion < VK_API_VERSION_1_3) {
				if (strcmp(ext.extensionName, VK_EXT_PIPELINE_CREATION_CACHE_CONTROL_EXTENSION_NAME) == 0) {
					m_extensions.pipelineCreationCacheControl = m_pipelineCreationCacheControlFeatures.pipelineCreationCacheControl;
				}
				else if (strcmp(ext.extensionName, VK_EXT_PIPELINE_CREATION_FEEDBACK_EXTENSION_NAME) == 0) {
					m_extensions.pipelineCreationFeedback = 1;
				}
				else if (strcmp(ext.extensionName, VK_EXT_PRIVATE_DATA_EXTENSION_NAME) == 0) {
					m_extensions.privateData = m_privateDataFeatures.privateData;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SHADER_DEMOTE_TO_HELPER_INVOCATION_EXTENSION_NAME) == 0) {
					m_extensions.shaderDemoteToHelperInvocation = m_shaderDemoteToHelperInvocationFeatures.shaderDemoteToHelperInvocation;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SUBGROUP_SIZE_CONTROL_EXTENSION_NAME) == 0) {
					m_extensions.subgroupSizeControl = m_subgroupSizeControlFeatures.subgroupSizeControl;
				}
				else if (strcmp(ext.extensionName, VK_EXT_TEXEL_BUFFER_ALIGNMENT_EXTENSION_NAME) == 0) {
					m_extensions.texelBufferAlignment = m_texelBufferAlignmentFeatures.texelBufferAlignment;
				}
				else if (strcmp(ext.extensionName, VK_EXT_TEXTURE_COMPRESSION_ASTC_HDR_EXTENSION_NAME) == 0) {
					m_extensions.textureCompressionAstcHdr = m_textureCompressionAstchdrFeatures.textureCompressionASTC_HDR;
				}
				else if (strcmp(ext.extensionName, VK_EXT_YCBCR_2PLANE_444_FORMATS_EXTENSION_NAME) == 0) {
					m_extensions.ycbcr2plane444Formats = m_ycbcr2Plane444FormatsFeatures.ycbcr2plane444Formats;
				}
				else if (strcmp(ext.extensionName, VK_EXT_IMAGE_ROBUSTNESS_EXTENSION_NAME) == 0) {
					m_extensions.imageRobustness = m_imageRobustnessFeatures.robustImageAccess;
				}
				else if (strcmp(ext.extensionName, VK_EXT_INLINE_UNIFORM_BLOCK_EXTENSION_NAME) == 0) {
					m_extensions.inlineUniformBlock = m_inlineUniformBlockFeatures.inlineUniformBlock;
				}
				else if (strcmp(ext.extensionName, VK_KHR_COPY_COMMANDS_2_EXTENSION_NAME) == 0) {
					m_extensions.copyCommands = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) == 0) {
					bool dependencies = true;
					if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
						dependencies &= use_extension(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
					dependencies &= m_extensions.depthStencilResolve;
					if (!dependencies)
						break;
					m_extensions.dynamicRendering = m_dynamicRenderingFeatures.dynamicRendering;
				}
				else if (strcmp(ext.extensionName, VK_KHR_MAINTENANCE_4_EXTENSION_NAME) == 0) {
					m_extensions.dynamicRendering = m_maintenance4Features.maintenance4;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_INTEGER_DOT_PRODUCT_EXTENSION_NAME) == 0) {
					m_extensions.dynamicRendering = m_shaderIntegerDotProductFeatures.shaderIntegerDotProduct;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME) == 0) {
					m_extensions.shaderNonSemanticInfo = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_TERMINATE_INVOCATION_EXTENSION_NAME) == 0) {
					m_extensions.shaderTerminateInvocation = m_shaderTerminateInvocationFeatures.shaderTerminateInvocation;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME) == 0) {
					m_extensions.synchronization2 = m_synchronization2Features.synchronization2;
				}
				else if (strcmp(ext.extensionName, VK_KHR_ZERO_INITIALIZE_WORKGROUP_MEMORY_EXTENSION_NAME) == 0) {
					m_extensions.zeroInitializeWorkgroupMemory = m_zeroInitializeWorkgroupMemoryFeatures.shaderZeroInitializeWorkgroupMemory;
				}
				else if (strcmp(ext.extensionName, VK_EXT_4444_FORMATS_EXTENSION_NAME) == 0) {
					m_extensions.formats4444 = m_4444FormatsFeatures.formatA4B4G4R4 && m_4444FormatsFeatures.formatA4R4G4B4;
				}
				else if (strcmp(ext.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
					m_extensions.extendedDynamicState = m_extendedDynamicStateFeatures.extendedDynamicState;
				}
				else if (strcmp(ext.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_2_EXTENSION_NAME) == 0) {
					m_extensions.extendedDynamicState2 = m_extendedDynamicState2Features.extendedDynamicState2;
				}
			}
			if (m_properties.properties.apiVersion < VK_API_VERSION_1_2) {
				if (strcmp(ext.extensionName, VK_KHR_8BIT_STORAGE_EXTENSION_NAME) == 0) {
					m_extensions.bit8Storage = m_8bitStorageFeatures.storageBuffer8BitAccess;
				}
				else if (strcmp(ext.extensionName, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME) == 0) {
					m_extensions.bufferDeviceAddress = m_bufferDeviceAddressFeatures.bufferDeviceAddress;
				}
				else if (strcmp(ext.extensionName, VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME) == 0) {
					m_extensions.createRenderpass2 = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) == 0) {
					bool dependencies = true;
					dependencies &= use_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
					dependencies &= m_extensions.createRenderpass2;
					if (!dependencies)
						break;
					m_extensions.depthStencilResolve = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_DRAW_INDIRECT_COUNT_EXTENSION_NAME) == 0) {
					m_extensions.drawIndirectCount = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_DRIVER_PROPERTIES_EXTENSION_NAME) == 0) {
					m_extensions.driverProperties = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME) == 0) {
					m_extensions.imageFormatList = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_IMAGELESS_FRAMEBUFFER_EXTENSION_NAME) == 0) {
					bool dependencies = true;
					dependencies &= use_extension(VK_KHR_IMAGE_FORMAT_LIST_EXTENSION_NAME);
					dependencies &= m_extensions.imagelessFramebuffer;
					if (!dependencies)
						break;
					m_extensions.imagelessFramebuffer = m_imagelessFramebufferFeatures.imagelessFramebuffer;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SAMPLER_MIRROR_CLAMP_TO_EDGE_EXTENSION_NAME) == 0) {
					m_extensions.samplerMirrorClampToEdge = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SEPARATE_DEPTH_STENCIL_LAYOUTS_EXTENSION_NAME) == 0) {
					bool dependencies = true;
					dependencies &= use_extension(VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME);
					dependencies &= m_extensions.createRenderpass2;
					if (!dependencies)
						break;
					m_extensions.separateDepthStencilLayouts = m_separateDepthStencilLayoutsFeatures.separateDepthStencilLayouts;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_ATOMIC_INT64_EXTENSION_NAME) == 0) {
					m_extensions.shaderAtomicInt64 = m_shaderAtomicInt64Features.shaderSharedInt64Atomics && m_shaderAtomicInt64Features.shaderBufferInt64Atomics;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME) == 0) {
					m_extensions.shaderFloat16Int8 = m_shaderFloat16Int8Features.shaderFloat16 && m_shaderFloat16Int8Features.shaderInt8;
					m_extensions.float16Int8 = m_float16Int8Features.shaderFloat16 && m_float16Int8Features.shaderInt8;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME) == 0) {
					m_extensions.shaderFloatControls = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SHADER_SUBGROUP_EXTENDED_TYPES_EXTENSION_NAME) == 0) {
					m_extensions.shaderSubgroupExtendedTypes = m_shaderSubgroupExtendedTypesFeatures.shaderSubgroupExtendedTypes;
				}
				else if (strcmp(ext.extensionName, VK_KHR_SPIRV_1_4_EXTENSION_NAME) == 0) {
					bool dependencies = true;
					dependencies &= use_extension(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME);
					dependencies &= m_extensions.shaderFloatControls;
					if (!dependencies)
						break;
					m_extensions.spirv14 = 1;
				}
				else if (strcmp(ext.extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0) {
					m_extensions.timelineSemaphore = m_timelineSemaphoreFeatures.timelineSemaphore;
				}
				else if (strcmp(ext.extensionName, VK_KHR_UNIFORM_BUFFER_STANDARD_LAYOUT_EXTENSION_NAME) == 0) {
					m_extensions.uniformBufferStandardLayout = m_uniformBufferStandardLayoutFeatures.uniformBufferStandardLayout;
				}
				else if (strcmp(ext.extensionName, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME) == 0) {
					m_extensions.vulkanMemoryModel = m_vulkanMemoryModelFeatures.vulkanMemoryModel;
				}
				else if (strcmp(ext.extensionName, VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME) == 0) {
					m_extensions.descriptorIndexing = m_descriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing
						&& m_descriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing
						&& m_descriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing
						&& m_descriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing
						&& m_descriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing
						&& m_descriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing
						&& m_descriptorIndexingFeatures.shaderStorageTexelBufferArrayNonUniformIndexing
						&& m_descriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind
						&& m_descriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind
						&& m_descriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind
						&& m_descriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind
						&& m_descriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind
						&& m_descriptorIndexingFeatures.descriptorBindingUpdateUnusedWhilePending
						&& m_descriptorIndexingFeatures.descriptorBindingPartiallyBound
						&& m_descriptorIndexingFeatures.runtimeDescriptorArray;
				}
				else if (strcmp(ext.extensionName, VK_EXT_HOST_QUERY_RESET_EXTENSION_NAME) == 0) {
					m_extensions.hostQueryReset = m_hostQueryResetFeatures.hostQueryReset;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SAMPLER_FILTER_MINMAX_EXTENSION_NAME) == 0) {
					m_extensions.samplerFilterMinmax = 1;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SCALAR_BLOCK_LAYOUT_EXTENSION_NAME) == 0) {
					m_extensions.scalarBlockLayout = m_scalarBlockLayoutFeatures.scalarBlockLayout;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SEPARATE_STENCIL_USAGE_EXTENSION_NAME) == 0) {
					m_extensions.separateStencilUsage = 1;
				}
				else if (strcmp(ext.extensionName, VK_EXT_SHADER_VIEWPORT_INDEX_LAYER_EXTENSION_NAME) == 0) {
					m_extensions.shaderViewportIndexLayer = 1;
				}
			}
			if (std::find(std::begin(m_usedExtensions), std::end(m_usedExtensions), extension) == std::end(m_usedExtensions))
				m_usedExtensions.push_back(extension);
			return true;
		}
	}
	Utility::log().location().format("Requested device extension not available: {}", extension);
	return false;
}

std::unique_ptr<vulkan::LogicalDevice> vulkan::PhysicalDevice::create_logical_device(const Instance& instance)
{
	float priority{ 1.f };
	std::vector< VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(3);
	queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_genericQueueFamily,
		.queueCount = static_cast<uint32_t>(1),
		.pQueuePriorities = &priority
		});
	if (m_computeQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_computeQueueFamily,
			.queueCount = static_cast<uint32_t>(1),
			.pQueuePriorities = &priority
			});
	}
	if (m_transferQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_transferQueueFamily,
			.queueCount = static_cast<uint32_t>(1),
			.pQueuePriorities = &priority
			});
	}
	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &m_features,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(m_usedExtensions.size()),
		.ppEnabledExtensionNames = m_usedExtensions.data(),
		.pEnabledFeatures = nullptr,
	};
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		createInfo.pNext = nullptr;
		createInfo.pEnabledFeatures = &m_features.features;
	}
	update_feature_chain();
	VkDevice logicalDevice {VK_NULL_HANDLE};
	if (auto result = vkCreateDevice(m_vkHandle, &createInfo, nullptr, &logicalDevice); result != VK_SUCCESS) {
		if (result == VK_ERROR_DEVICE_LOST) {
			throw Utility::DeviceLostException("Could not create device");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return std::make_unique<LogicalDevice>(instance, *this, logicalDevice, m_genericQueueFamily, m_computeQueueFamily, m_transferQueueFamily);
}


std::unique_ptr<vulkan::LogicalDevice> vulkan::PhysicalDevice::create_logical_device(const Instance& instance, const std::vector<float>& genericQueuePriorities, const std::vector<float>& computeQueuePriorities, const std::vector<float>& transferQueuePriorities)
{
	std::vector< VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(3);
	queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_genericQueueFamily,
		.queueCount = static_cast<uint32_t>(genericQueuePriorities.size()),
		.pQueuePriorities = genericQueuePriorities.data()
		});
	if (m_computeQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_computeQueueFamily,
			.queueCount = static_cast<uint32_t>(computeQueuePriorities.size()),
			.pQueuePriorities = computeQueuePriorities.data()
			});
	}
	if (m_transferQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_transferQueueFamily,
			.queueCount = static_cast<uint32_t>(transferQueuePriorities.size()),
			.pQueuePriorities = transferQueuePriorities.data()
			});
	}
	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &m_features,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(m_usedExtensions.size()),
		.ppEnabledExtensionNames = m_usedExtensions.data(),
		.pEnabledFeatures = nullptr,
	};

	if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		createInfo.pNext = nullptr;
		createInfo.pEnabledFeatures = &m_features.features;
	}
	VkDevice logicalDevice;
	if (auto result = vkCreateDevice(m_vkHandle, &createInfo, nullptr, &logicalDevice); result != VK_SUCCESS) {
		if (result == VK_ERROR_DEVICE_LOST) {
			throw Utility::DeviceLostException("Could not create device");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	vulkan::LogicalDevice::QueueInfos infos {
		.graphicsFamilyQueueIndex {m_genericQueueFamily},
		.computeFamilyQueueIndex {m_computeQueueFamily},
		.transferFamilyQueueIndex {m_transferQueueFamily},
		//.graphicsPresentCapable {supports_surface(surface, m_genericQueueFamily)},
		//.computePresentCapable {supports_surface(surface, m_computeQueueFamily)},
		//.transferPresentCapable {supports_surface(surface, m_transferQueueFamily)},
		.graphicsQueuePriorities{genericQueuePriorities},
		.computeQueuePriorities {computeQueuePriorities},
		.transferQueuePriorities{transferQueuePriorities},
	};
	return std::make_unique<LogicalDevice>(instance, *this, logicalDevice, infos);
}

bool vulkan::PhysicalDevice::supports_surface(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const
{
	if (surface == VK_NULL_HANDLE || queueFamilyIndex >= m_queueFamilyProperties.size())//VUID-vkGetPhysicalDeviceSurfaceSupportKHR-queueFamilyIndex-01269
		return false;
	
	VkBool32 supported{};
	if (const auto result = vkGetPhysicalDeviceSurfaceSupportKHR(m_vkHandle, queueFamilyIndex, surface, &supported); result != VK_SUCCESS) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfaceSupportKHR: Could not query surface support");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return supported;
}

std::vector<VkPresentModeKHR> vulkan::PhysicalDevice::get_present_modes(VkSurfaceKHR surface) const
{
	if (surface == VK_NULL_HANDLE)
		return {};
	uint32_t numModes{ 0 };
	if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkHandle, surface, &numModes, nullptr); result != VK_SUCCESS && result != VK_INCOMPLETE) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfacePresentModesKHR: Could not query surface present modes");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	std::vector<VkPresentModeKHR> presentModes(numModes);
	if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(m_vkHandle, surface, &numModes, presentModes.data()); result != VK_SUCCESS && result != VK_INCOMPLETE) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfacePresentModesKHR: Could not query surface present modes");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return presentModes;
}

std::vector<VkSurfaceFormat2KHR> vulkan::PhysicalDevice::get_surface_formats2(VkSurfaceKHR surface) const
{
	if (surface == VK_NULL_HANDLE)
		return {};
	uint32_t numFormats{ 0 };
	VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreenExclusiveWin32Info{
		.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT},
		.pNext {nullptr},
		.hmonitor {}
	};
	VkSurfaceFullScreenExclusiveInfoEXT fullscreenExclusiveInfo{
		.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT},
		.pNext {nullptr},
		.fullScreenExclusive {VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT }
	};
	VkSurfacePresentModeEXT presentModeInfo{
		.sType {VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT},
		.pNext {nullptr},
		.presentMode {VK_PRESENT_MODE_IMMEDIATE_KHR}
	};
	VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{
		.sType {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR},
		.pNext {nullptr},
		.surface {surface}
	};
	if (auto result = vkGetPhysicalDeviceSurfaceFormats2KHR(m_vkHandle, &surfaceInfo, &numFormats, nullptr); result != VK_SUCCESS && result != VK_INCOMPLETE) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfaceFormats2KHR: Could not query surface formats");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	std::vector<VkSurfaceFormat2KHR> surfaceFormats(numFormats, VkSurfaceFormat2KHR{ .sType {VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR},.pNext {nullptr} });
	if (auto result = vkGetPhysicalDeviceSurfaceFormats2KHR(m_vkHandle, &surfaceInfo, &numFormats, surfaceFormats.data()); result != VK_SUCCESS && result != VK_INCOMPLETE) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfaceFormats2KHR: Could not query surface formats");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return surfaceFormats;
}

VkSurfaceCapabilities2KHR vulkan::PhysicalDevice::get_surface_capabilites2(VkSurfaceKHR surface) const
{
	VkSurfaceCapabilities2KHR capabilities{
		.sType {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR},
		.pNext {nullptr}
	};
	VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreenExclusiveWin32Info{
		.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT},
		.pNext {nullptr},
		.hmonitor {}
	};
	VkSurfaceFullScreenExclusiveInfoEXT fullscreenExclusiveInfo{
		.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT},
		.pNext {nullptr},
		.fullScreenExclusive {VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT }
	};
	VkSurfacePresentModeEXT presentModeInfo{
		.sType {VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT},
		.pNext {nullptr},
		.presentMode {}
	};
	VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{
		.sType {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR},
		.pNext {nullptr},
		.surface {surface}
	};
	if (auto result = vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_vkHandle, &surfaceInfo, &capabilities); result != VK_SUCCESS) {
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfaceCapabilities2KHR: Could not query surface formats");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return capabilities;
}

const std::vector<const char*>& vulkan::PhysicalDevice::get_used_extensions() const noexcept
{
	return m_usedExtensions;
}

const std::vector<VkQueueFamilyProperties>& vulkan::PhysicalDevice::get_queue_family_properties() const noexcept
{
	return m_queueFamilyProperties;
}

const VkPhysicalDeviceFeatures& vulkan::PhysicalDevice::get_used_features() const noexcept
{
	return m_features.features;
}

const VkPhysicalDeviceVulkan11Features& vulkan::PhysicalDevice::get_vulkan11_features() const noexcept
{
	return m_11Features;
}

const VkPhysicalDeviceVulkan12Features& vulkan::PhysicalDevice::get_vulkan12_features() const noexcept
{
	return m_12Features;
}

const VkPhysicalDeviceVulkan13Features& vulkan::PhysicalDevice::get_vulkan13_features() const noexcept
{
	return m_13Features;
}

const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& vulkan::PhysicalDevice::get_atomic_float_features() const noexcept
{
	return m_atomicFloatFeatures;
}

const VkPhysicalDeviceAccelerationStructureFeaturesKHR& vulkan::PhysicalDevice::get_acceleration_structure_features() const noexcept
{
	return m_accelerationStructureFeatures;
}

const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& vulkan::PhysicalDevice::get_ray_tracing_pipeline_features() const noexcept
{
	return m_rayTracingPipelineFeatures;
}

const VkPhysicalDeviceRayQueryFeaturesKHR& vulkan::PhysicalDevice::get_ray_query_features() const noexcept
{
	return m_rayQueryFeatures;
}

const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& vulkan::PhysicalDevice::get_vertex_input_dynamic_state_features() const noexcept
{
	return m_vertexInputDynamicStateFeatures;
}

const VkPhysicalDeviceMeshShaderFeaturesEXT& vulkan::PhysicalDevice::get_mesh_shader_features() const noexcept
{
	return m_meshShaderFeatures;
}

const VkPhysicalDeviceProperties& vulkan::PhysicalDevice::get_properties() const noexcept
{
	return m_properties.properties;
}

const VkPhysicalDeviceSubgroupProperties& vulkan::PhysicalDevice::get_subgroup_properties() const noexcept
{
	return m_subgroupProperties;
}

const VkPhysicalDeviceVulkan11Properties& vulkan::PhysicalDevice::get_vulkan11_properties() const noexcept
{
	return m_11Properties;
}

const VkPhysicalDeviceVulkan12Properties& vulkan::PhysicalDevice::get_vulkan12_properties() const noexcept
{
	return m_12Properties;
}

const VkPhysicalDeviceVulkan13Properties& vulkan::PhysicalDevice::get_vulkan13_properties() const noexcept
{
	return m_13Properties;
}

const VkPhysicalDeviceAccelerationStructurePropertiesKHR& vulkan::PhysicalDevice::get_acceleration_structure_properties() const noexcept
{
	return m_accelerationStructureProperties;
}

const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& vulkan::PhysicalDevice::get_ray_tracing_pipeline_properties() const noexcept
{
	return m_rayTracingPipelineProperties;
}

const VkPhysicalDeviceMeshShaderPropertiesEXT& vulkan::PhysicalDevice::get_mesh_shader_properties() const noexcept
{
	return m_meshShaderProperties;
}

const vulkan::PhysicalDevice::Extensions& vulkan::PhysicalDevice::get_extensions() const noexcept
{
	return m_extensions;
}

std::optional<VkImageFormatProperties> vulkan::PhysicalDevice::get_image_format_properties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const noexcept
{
	VkImageFormatProperties properties;
	if (const auto result = vkGetPhysicalDeviceImageFormatProperties(m_vkHandle, format, type, tiling, usage, flags, &properties); result == VK_ERROR_FORMAT_NOT_SUPPORTED ||
		result == VK_ERROR_OUT_OF_HOST_MEMORY ||
		result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
		return std::nullopt;

	return properties;
}

void vulkan::PhysicalDevice::init_queues() noexcept
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkHandle, &numQueueFamilies, nullptr);
	m_queueFamilyProperties.resize(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkHandle, &numQueueFamilies, m_queueFamilyProperties.data());

	for (uint32_t i = 0; i < m_queueFamilyProperties.size(); i++) {
		const auto& queue = m_queueFamilyProperties[i];
		if ((queue.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)) == VK_QUEUE_TRANSFER_BIT) {
			if(m_transferQueueFamily == ~0u)
			m_transferQueueFamily = i;
		}
		else if ((queue.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT)) == VK_QUEUE_COMPUTE_BIT) {
			m_computeQueueFamily = i;
		}
		else if ((queue.queueFlags & VK_QUEUE_GRAPHICS_BIT) == VK_QUEUE_GRAPHICS_BIT) {
			m_genericQueueFamily = i;
		}
	}
}

void vulkan::PhysicalDevice::init_extensions() noexcept
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(m_vkHandle, nullptr, &extensionCount, nullptr);
	m_availableExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_vkHandle, nullptr, &extensionCount, m_availableExtensions.data());
}

void vulkan::PhysicalDevice::init_features() noexcept
{
	vkGetPhysicalDeviceProperties(m_vkHandle, &m_properties.properties);
	if(m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		vkGetPhysicalDeviceFeatures(m_vkHandle, &m_features.features);
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
		m_shaderAtomicInt64Features.pNext = &m_float16Int8Features;
		m_float16Int8Features.pNext = &m_shaderFloat16Int8Features;
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

	vkGetPhysicalDeviceFeatures2(m_vkHandle, &m_features);

	m_features.pNext = nullptr;


	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_3)
	{
		m_extensions.copyCommands = 1;
		m_extensions.dynamicRendering = m_13Features.dynamicRendering;
		m_extensions.formatFeatureFlags2 = 1;
		m_extensions.maintenance4 = m_13Features.maintenance4;
		m_extensions.shaderIntegerDotProduct = m_13Features.shaderIntegerDotProduct;
		m_extensions.shaderNonSemanticInfo = 1;
		m_extensions.shaderTerminateInvocation = m_13Features.shaderTerminateInvocation;
		m_extensions.synchronization2 = m_13Features.synchronization2;
		m_extensions.zeroInitializeWorkgroupMemory = m_13Features.shaderZeroInitializeWorkgroupMemory;
		m_extensions.formats4444 = 1;
		m_extensions.extendedDynamicState = 1;
		m_extensions.extendedDynamicState2 = 1;
		m_extensions.imageRobustness = m_13Features.robustImageAccess;
		m_extensions.inlineUniformBlock = m_13Features.inlineUniformBlock;
		m_extensions.pipelineCreationCacheControl = m_13Features.pipelineCreationCacheControl;
		m_extensions.pipelineCreationFeedback = 1;
		m_extensions.privateData = m_13Features.privateData;
		m_extensions.shaderDemoteToHelperInvocation = m_13Features.shaderDemoteToHelperInvocation;
		m_extensions.subgroupSizeControl = m_13Features.subgroupSizeControl;
		m_extensions.texelBufferAlignment = 1;
		m_extensions.textureCompressionAstcHdr = m_13Features.textureCompressionASTC_HDR;
		m_extensions.toolingInfo = 1;
		m_extensions.ycbcr2plane444Formats = 1;
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_2)
	{
		m_extensions.bit8Storage = m_12Features.storageBuffer8BitAccess && m_12Features.uniformAndStorageBuffer8BitAccess && m_12Features.storagePushConstant8;
		m_extensions.bufferDeviceAddress = m_12Features.bufferDeviceAddress;
		m_extensions.createRenderpass2 = 1;
		m_extensions.depthStencilResolve = 1;
		m_extensions.drawIndirectCount = m_12Features.drawIndirectCount;
		m_extensions.driverProperties = 1;
		m_extensions.imageFormatList = 1;
		m_extensions.imagelessFramebuffer = m_12Features.imagelessFramebuffer;
		m_extensions.samplerMirrorClampToEdge = m_12Features.samplerMirrorClampToEdge;
		m_extensions.separateDepthStencilLayouts = m_12Features.separateDepthStencilLayouts;
		m_extensions.shaderAtomicInt64 = m_12Features.shaderBufferInt64Atomics && m_12Features.shaderSharedInt64Atomics;
		m_extensions.shaderFloat16Int8 = m_12Features.shaderFloat16 && m_12Features.shaderInt8;
		m_extensions.float16Int8 = m_12Features.shaderFloat16 && m_12Features.shaderInt8;
		m_extensions.shaderFloatControls = 1;
		m_extensions.shaderSubgroupExtendedTypes = m_12Features.shaderSubgroupExtendedTypes;
		m_extensions.spirv14 = 1;
		m_extensions.timelineSemaphore = m_12Features.timelineSemaphore;
		m_extensions.uniformBufferStandardLayout = m_12Features.uniformBufferStandardLayout;
		m_extensions.vulkanMemoryModel = m_12Features.vulkanMemoryModel;
		m_extensions.descriptorIndexing = m_12Features.descriptorIndexing;
		m_extensions.hostQueryReset = m_12Features.hostQueryReset;
		m_extensions.samplerFilterMinmax = m_12Features.samplerFilterMinmax;
		m_extensions.scalarBlockLayout = m_12Features.scalarBlockLayout;
		m_extensions.separateStencilUsage = 1;
		m_extensions.shaderViewportIndexLayer = m_12Features.shaderOutputViewportIndex;
	}
}

void vulkan::PhysicalDevice::init_properties() noexcept
{
	vkGetPhysicalDeviceProperties(m_vkHandle, &m_properties.properties);
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

	vkGetPhysicalDeviceProperties2(m_vkHandle, &m_properties);
}

void vulkan::PhysicalDevice::update_feature_chain() noexcept
{
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_1) 
	{
		m_11Features.pNext = m_features.pNext;
		m_features.pNext = &m_11Features;
	} else
	{
		assert(false);
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_2) 
	{
		m_12Features.pNext = m_features.pNext;
		m_features.pNext = &m_12Features;
	} else
	{
		if(m_extensions.bit8Storage) {
			m_8bitStorageFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_8bitStorageFeatures;
		}
		if(m_extensions.bufferDeviceAddress) {
			m_bufferDeviceAddressFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_bufferDeviceAddressFeatures;
		}
		if(m_extensions.separateDepthStencilLayouts) {
			m_separateDepthStencilLayoutsFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_separateDepthStencilLayoutsFeatures;
		}
		if(m_extensions.shaderAtomicInt64) {
			m_shaderAtomicInt64Features.pNext =m_features.pNext;
			m_features.pNext = &m_shaderAtomicInt64Features;
		}
		if(m_extensions.float16Int8) {
			m_float16Int8Features.pNext =m_features.pNext;
			m_features.pNext = &m_float16Int8Features;
		}
		if(m_extensions.shaderFloat16Int8) {
			m_shaderFloat16Int8Features.pNext =m_features.pNext;
			m_features.pNext = &m_shaderFloat16Int8Features;
		}
		if(m_extensions.shaderSubgroupExtendedTypes) {
			m_shaderSubgroupExtendedTypesFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_shaderSubgroupExtendedTypesFeatures;
		}
		if(m_extensions.timelineSemaphore) {
			m_timelineSemaphoreFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_timelineSemaphoreFeatures;
		}
		if(m_extensions.uniformBufferStandardLayout) {
			m_uniformBufferStandardLayoutFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_uniformBufferStandardLayoutFeatures;
		}
		if(m_extensions.vulkanMemoryModel) {
			m_vulkanMemoryModelFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_vulkanMemoryModelFeatures;
		}
		if(m_extensions.descriptorIndexing) {
			m_descriptorIndexingFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_descriptorIndexingFeatures;
		}
		if(m_extensions.hostQueryReset) {
			m_hostQueryResetFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_hostQueryResetFeatures;
		}
		if(m_extensions.scalarBlockLayout) {
			m_scalarBlockLayoutFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_scalarBlockLayoutFeatures;
		}
		if(m_extensions.imagelessFramebuffer) {
			m_imagelessFramebufferFeatures.pNext =m_features.pNext;
			m_features.pNext = &m_imagelessFramebufferFeatures;
		}
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_3) 
	{
		m_13Features.pNext = m_features.pNext;
		m_features.pNext = &m_13Features;
	} else
	{
		if (m_extensions.pipelineCreationCacheControl) {
			m_pipelineCreationCacheControlFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_pipelineCreationCacheControlFeatures;
		}
		if(m_extensions.privateData) {
			m_privateDataFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_privateDataFeatures;
		}
		if(m_extensions.shaderDemoteToHelperInvocation) {
			m_shaderDemoteToHelperInvocationFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderDemoteToHelperInvocationFeatures;
		}
		if(m_extensions.subgroupSizeControl) {
			m_subgroupSizeControlFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_subgroupSizeControlFeatures;
		}
		if(m_extensions.texelBufferAlignment) {
			m_texelBufferAlignmentFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_texelBufferAlignmentFeatures;
		}
		if(m_extensions.textureCompressionAstcHdr) {
			m_textureCompressionAstchdrFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_textureCompressionAstchdrFeatures;
		}
		if(m_extensions.imageRobustness) {
			m_imageRobustnessFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_imageRobustnessFeatures;
		}
		if(m_extensions.inlineUniformBlock) {
			m_inlineUniformBlockFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_inlineUniformBlockFeatures;
		}
		if(m_extensions.dynamicRendering) {
			m_dynamicRenderingFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_dynamicRenderingFeatures;
		}
		if(m_extensions.maintenance4) {
			m_maintenance4Features.pNext = m_features.pNext;
			m_features.pNext = &m_maintenance4Features;
		}
		if(m_extensions.shaderIntegerDotProduct) {
			m_shaderIntegerDotProductFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderIntegerDotProductFeatures;
		}
		if(m_extensions.shaderTerminateInvocation) {
			m_shaderTerminateInvocationFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_shaderTerminateInvocationFeatures;
		}
		if(m_extensions.synchronization2) {
			m_synchronization2Features.pNext = m_features.pNext;
			m_features.pNext = &m_synchronization2Features;
		}
		if(m_extensions.zeroInitializeWorkgroupMemory) {
			m_zeroInitializeWorkgroupMemoryFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_zeroInitializeWorkgroupMemoryFeatures;
		}
		if(m_extensions.formats4444) {
			m_4444FormatsFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_4444FormatsFeatures;
		}
		if(m_extensions.extendedDynamicState) {
			m_extendedDynamicStateFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_extendedDynamicStateFeatures;
		}
		if(m_extensions.extendedDynamicState2) {
			m_extendedDynamicState2Features.pNext = m_features.pNext;
			m_features.pNext = &m_extendedDynamicState2Features;
		}
		if(m_extensions.ycbcr2plane444Formats) {
			m_ycbcr2Plane444FormatsFeatures.pNext = m_features.pNext;
			m_features.pNext = &m_ycbcr2Plane444FormatsFeatures;
		}
	}

	if (m_extensions.acceleration_structure) 
	{
		m_accelerationStructureFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_accelerationStructureFeatures;
	}
	if (m_extensions.ray_tracing_pipeline) 
	{
		m_rayTracingPipelineFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_rayTracingPipelineFeatures;
	}
	if (m_extensions.ray_query) 
	{
		m_rayQueryFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_rayQueryFeatures;
	}
	if (m_extensions.vertex_input_dynamic_state) 
	{
		m_vertexInputDynamicStateFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_vertexInputDynamicStateFeatures;
	}
	if (m_extensions.mesh_shader) 
	{
		m_meshShaderFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_meshShaderFeatures;
	}
	if (m_extensions.atomic_floats) 
	{
		m_atomicFloatFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_atomicFloatFeatures;
	}
	if (m_extensions.present_id) 
	{
		m_presentIdFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_presentIdFeatures;
	}
	if (m_extensions.present_wait) 
	{
		m_presentWaitFeatures.pNext = m_features.pNext;
		m_features.pNext = &m_presentWaitFeatures;
	}
}
