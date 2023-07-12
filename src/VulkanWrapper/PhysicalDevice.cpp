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
					return false;
				m_extensions.present_id = m_presentIdFeatures.presentId;
			}
			else if (strcmp(ext.extensionName, VK_KHR_PRESENT_WAIT_EXTENSION_NAME) == 0) {
				bool dependencies = true;
				dependencies &= use_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				dependencies &= use_extension(VK_KHR_PRESENT_ID_EXTENSION_NAME);
				if (!dependencies)
					return false;
				m_extensions.present_wait = m_presentWaitFeatures.presentWait;
			}
			else if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
				m_extensions.swapchain = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
				bool dependencies = true;
				dependencies &= use_extension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
				if (!dependencies)
					return false;
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
	m_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	m_features.pNext = &m_11Features;

	m_11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	m_11Features.pNext = &m_12Features;

	m_12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	m_12Features.pNext = &m_13Features;


	m_13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	m_13Features.pNext = &m_atomicFloatFeatures;

	m_atomicFloatFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT;
	m_atomicFloatFeatures.pNext = &m_accelerationStructureFeatures;

	m_accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	m_accelerationStructureFeatures.pNext = &m_rayTracingPipelineFeatures;

	m_rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	m_rayTracingPipelineFeatures.pNext = &m_rayQueryFeatures;

	m_rayQueryFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR;
	m_rayQueryFeatures.pNext = &m_vertexInputDynamicStateFeatures;

	m_vertexInputDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
	m_vertexInputDynamicStateFeatures.pNext = &m_meshShaderFeatures;

	m_meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
	m_meshShaderFeatures.pNext = &m_presentIdFeatures;

	m_presentIdFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_ID_FEATURES_KHR;
	m_presentIdFeatures.pNext = &m_presentWaitFeatures;

	m_presentWaitFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PRESENT_WAIT_FEATURES_KHR;
	m_presentWaitFeatures.pNext = nullptr;


	vkGetPhysicalDeviceFeatures2(m_vkHandle, &m_features);

	m_features.pNext = nullptr;
}

void vulkan::PhysicalDevice::init_properties() noexcept
{
	vkGetPhysicalDeviceProperties(m_vkHandle, &m_properties.properties);
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_1)
	{
		return;
	}
	m_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	m_properties.pNext = &m_subgroupProperties;

	m_subgroupProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_PROPERTIES;
	m_subgroupProperties.pNext = &m_11Properties;

	m_11Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	m_11Properties.pNext = &m_12Properties;
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_2)
		m_11Properties.pNext = &m_accelerationStructureProperties;

	m_12Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
	m_12Properties.pNext = &m_13Properties;
	if (m_properties.properties.apiVersion < VK_API_VERSION_1_3)
		m_12Properties.pNext = &m_accelerationStructureProperties;

	m_13Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
	m_13Properties.pNext = &m_accelerationStructureProperties;

	m_accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	m_accelerationStructureProperties.pNext = &m_rayTracingPipelineProperties;

	m_rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	m_rayTracingPipelineProperties.pNext = &m_meshShaderProperties;

	m_meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
	m_meshShaderProperties.pNext = nullptr;

	vkGetPhysicalDeviceProperties2(m_vkHandle, &m_properties);
}

void vulkan::PhysicalDevice::update_feature_chain() noexcept
{
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_1) 
	{
		m_11Features.pNext = m_features.pNext;
		m_features.pNext = &m_11Features;
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_2) 
	{
		m_12Features.pNext = m_features.pNext;
		m_features.pNext = &m_12Features;
	}
	if (m_properties.properties.apiVersion >= VK_API_VERSION_1_3) 
	{
		m_13Features.pNext = m_features.pNext;
		m_features.pNext = &m_13Features;
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
