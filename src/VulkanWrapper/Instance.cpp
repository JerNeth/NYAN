#include "Instance.h"
#include <stdexcept>

#include "Utility/Exceptions.h"
#include "LogicalDevice.h"


[[maybe_unused]] static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	Utility::log_error().format("[vulkan] Debug report from ObjectType: {}\nMessage:{}\n\n", static_cast<int>(objectType), pMessage);
	return VK_FALSE;
}

vulkan::Instance::Instance(const char** extensions, uint32_t extensionCount, std::string applicationName, std::string engineName)
	: m_applicationName(applicationName),
	m_engineName(engineName) 
{
	volkInitialize();
	m_extensions.assign(extensions, extensions + extensionCount);
	create_instance();

}

vulkan::Instance::~Instance() {
	if constexpr (debug) {
		//auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
		vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, m_allocator);
	}
	vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
	vkDestroyInstance(m_instance, m_allocator);

}
std::unique_ptr<vulkan::LogicalDevice> vulkan::Instance::setup_device(const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions)
{
	uint32_t numDevices;
	std::vector<VkPhysicalDevice> devices;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);
	if (numDevices == 0) {
		throw std::runtime_error("VK: no physical device with Vulkan support available");
	}
	
	devices.resize(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, devices.data());
	m_bestDevice = 0;
	int bestDeviceOptCount = 0;
	for (size_t i = 0; i < numDevices; i++) {
		m_physicalDevices.emplace_back(devices[i]);
		bool required = true;
		int optional = 0;
		auto& dev = m_physicalDevices.back();
		for (const auto& extension : requiredExtensions)
			required &= dev.use_extension(extension);
		if (!required)
			continue;
		for (const auto& extension : optionalExtensions)
			optional += dev.use_extension(extension);
		if (optional > bestDeviceOptCount) {
			m_bestDevice = i;
			bestDeviceOptCount = optional;
		}

	}
	m_physicalDevice = m_physicalDevices[m_bestDevice].get_handle();

	return m_physicalDevices[m_bestDevice].create_logical_device(*this);
}

#ifdef WIN32
void vulkan::Instance::setup_win32_surface(HWND hwnd, HINSTANCE hinstance) {
	VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hinstance,
		.hwnd = hwnd,
	};
	if (auto result = vkCreateWin32SurfaceKHR(m_instance, &createInfo, m_allocator, &m_surface); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}
#else
void vulkan::Instance::setup_x11_surface(Window window, Display* dpy) {
	VkXlibSurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
		.dpy = dpy,
		.window = window,
	};
	if (auto result = vkCreateXlibSurfaceKHR(m_instance, &createInfo, m_allocator, &m_surface); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}
#endif

std::vector<VkPresentModeKHR> vulkan::Instance::get_present_modes() const
{
	uint32_t numModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numModes, nullptr);
	std::vector<VkPresentModeKHR> presentModes(numModes);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numModes, presentModes.data());
	return presentModes;
}

std::vector<VkSurfaceFormatKHR> vulkan::Instance::get_surface_formats() const
{
	uint32_t numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numFormats, nullptr);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numFormats, surfaceFormats.data());
	return surfaceFormats;
}

VkSurfaceCapabilitiesKHR vulkan::Instance::get_surface_capabilites() const
{
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);
	return capabilities;
}

VkSurfaceKHR vulkan::Instance::get_surface() const
{
	return m_surface;
}

vulkan::Instance::operator VkInstance() const noexcept
{
	return m_instance;
}

const vulkan::PhysicalDevice& vulkan::Instance::get_physical_device() const noexcept
{
	return m_physicalDevices[m_bestDevice];
}

void vulkan::Instance::create_instance(uint32_t applicationVersion, uint32_t engineVersion)
{
	VkApplicationInfo applicationInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = m_applicationName.c_str(),
				.applicationVersion = applicationVersion,
				.pEngineName = m_engineName.c_str(),
				.engineVersion = engineVersion,
				.apiVersion = VK_API_VERSION_1_3
	};

	if constexpr (debug) {
		m_extensions.push_back("VK_EXT_debug_report");
		m_extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		m_layers.push_back("VK_LAYER_KHRONOS_validation");
	}
	uint32_t layerPropertyCount;
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
	std::vector<VkLayerProperties> layerProperties(layerPropertyCount);
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties.data());

	for (auto it = m_layers.begin(); it != m_layers.end();) {
		if (std::find_if(layerProperties.begin(), layerProperties.end(), [&it](auto& val) {return std::strcmp(*it, val.layerName) == 0; }) == layerProperties.end()) {
			Utility::log().location().format("Instance layer not present: {}", *it);
			it = m_layers.erase(it);
		}
		else
			++it;
	}
	std::vector<const char*> extensions;

	{
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(NULL, &propertyCount, properties.data());

		for (auto it = m_extensions.begin(); it != m_extensions.end(); ++it)
			if (std::find_if(properties.begin(), properties.end(), [&it](auto& val) {return std::strcmp(*it, val.extensionName) == 0; }) != properties.end())
				extensions.push_back(*it);
	}

	for (auto& layer : m_layers) {
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, properties.data());

		for (auto it = m_extensions.begin(); it != m_extensions.end();++it)
			if (std::find_if(properties.begin(), properties.end(), [&it](auto& val) {return std::strcmp(*it, val.extensionName) == 0; }) != properties.end())
				extensions.push_back(*it);
	}

	for (auto it = m_extensions.begin(); it != m_extensions.end(); ++it) 
		if (std::find_if(extensions.begin(), extensions.end(), [&it](auto& val) {return std::strcmp(*it, val) == 0; }) == extensions.end())
			Utility::log().location().format("Instance extension not present: {}", *it);
	
	
	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(m_layers.size()),
		.ppEnabledLayerNames = m_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size()),
		.ppEnabledExtensionNames = m_extensions.data()
	};
	if (auto result = vkCreateInstance(&createInfo, m_allocator, &m_instance)) {
		throw Utility::VulkanException(result);
	}
	volkLoadInstance(m_instance);
	if constexpr (debug) {
		assert(vkCreateDebugReportCallbackEXT);
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
			.pfnCallback = debugCallback,
			.pUserData = nullptr
		};
		if (auto result = vkCreateDebugReportCallbackEXT(m_instance, &debugReportCallbackCreateInfo, m_allocator, &m_debugReport); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
	}
}

bool vulkan::Instance::device_swapchain_suitable(const VkPhysicalDevice& device) const
{
	uint32_t numFormats;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &numFormats, nullptr);
	if (numFormats == 0) {
		return false;
	}
	uint32_t numModes;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &numModes, nullptr);
	if (numModes == 0) {
		return false;
	}

	//vkGetPhysicalDeviceSurfaceSupportKHR(device, graphicsQueueFamilyIndex, m_surface, &presentSupport);
	return true;
}


bool vulkan::Instance::is_device_suitable(const VkPhysicalDevice& device) const
{
	return device_swapchain_suitable(device);
}

vulkan::PhysicalDevice::PhysicalDevice(VkPhysicalDevice device):
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
				m_accelerationStructureFeatures.pNext = m_features.pNext;
				m_features.pNext = &m_accelerationStructureFeatures;
				m_extensions.acceleration_structure = m_accelerationStructureFeatures.accelerationStructure;
			}
			else if (strcmp(ext.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
				m_rayTracingPipelineFeatures.pNext = m_features.pNext;
				m_features.pNext = &m_rayTracingPipelineFeatures;
				m_extensions.ray_tracing_pipeline = m_rayTracingPipelineFeatures.rayTracingPipeline;
			}
			else if (strcmp(ext.extensionName, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
				m_vertexInputDynamicStateFeatures.pNext = m_features.pNext;
				m_features.pNext = &m_vertexInputDynamicStateFeatures;
				m_extensions.vertex_input_dynamic_state = m_vertexInputDynamicStateFeatures.vertexInputDynamicState;
			}
			else if (strcmp(ext.extensionName, VK_NV_MESH_SHADER_EXTENSION_NAME) == 0) {
				m_meshShaderFeatures.pNext = m_features.pNext;
				m_features.pNext = &m_meshShaderFeatures;
				m_extensions.mesh_shader = m_meshShaderFeatures.meshShader && m_meshShaderFeatures.taskShader;
			}
			else if (strcmp(ext.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
				m_extensions.swapchain = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
				m_extensions.fullscreen_exclusive = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
				m_extensions.debug_marker = 1;
			}
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
				m_extensions.debug_utils = 1;
			}
			else if (strcmp(ext.extensionName, VK_KHR_RAY_QUERY_EXTENSION_NAME) == 0) {
				m_extensions.ray_query = 1;
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
			if(std::find(std::begin(m_usedExtensions), std::end(m_usedExtensions), extension) == std::end(m_usedExtensions))
				m_usedExtensions.push_back(extension);
			return true;
		}
	}
	Utility::log().format("Extension not supported: {}", extension);
	return false;
}

std::unique_ptr<vulkan::LogicalDevice> vulkan::PhysicalDevice::create_logical_device(const Instance& instance, uint32_t genericQueueCount, uint32_t transferQueueCount, uint32_t computeQueueCount)
{
	float queuePriority = 1.0f;
	std::vector< VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(3);
	queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = m_genericQueueFamily,
		.queueCount = genericQueueCount,
		.pQueuePriorities = &queuePriority
		});
	if (m_computeQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_computeQueueFamily,
			.queueCount = transferQueueCount,
			.pQueuePriorities = &queuePriority
			});
	}
	if (m_transferQueueFamily != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = m_transferQueueFamily,
			.queueCount = computeQueueCount,
			.pQueuePriorities = &queuePriority
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
		.pEnabledFeatures = NULL,
	};
	VkDevice logicalDevice;
	if (auto result = vkCreateDevice(m_vkHandle, &createInfo, nullptr, &logicalDevice); result != VK_SUCCESS) {
		if (result == VK_ERROR_DEVICE_LOST) {
			throw Utility::DeviceLostException("Could not create device");
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
	return std::make_unique<LogicalDevice>(instance,*this, logicalDevice, m_genericQueueFamily, m_computeQueueFamily, m_transferQueueFamily);
}

bool vulkan::PhysicalDevice::supports_ray_pipelines() const noexcept
{
	return m_rayTracingPipelineFeatures.rayTracingPipeline;
}

uint32_t vulkan::PhysicalDevice::get_generic_queue_family() const noexcept
{
	return m_genericQueueFamily;
}

uint32_t vulkan::PhysicalDevice::get_transfer_queue_family() const noexcept
{
	return m_transferQueueFamily;
}

uint32_t vulkan::PhysicalDevice::get_compute_queue_family() const noexcept
{
	return m_computeQueueFamily;
}

const std::vector<const char*>& vulkan::PhysicalDevice::get_used_extensions() const noexcept
{
	return m_usedExtensions;
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

const VkPhysicalDeviceAccelerationStructureFeaturesKHR& vulkan::PhysicalDevice::get_acceleration_structure_features() const noexcept
{
	return m_accelerationStructureFeatures;
}

const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& vulkan::PhysicalDevice::get_ray_tracing_pipeline_features() const noexcept
{
	return m_rayTracingPipelineFeatures;
}

const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& vulkan::PhysicalDevice::get_vertex_input_dynamic_state_features() const noexcept
{
	return m_vertexInputDynamicStateFeatures;
}

const VkPhysicalDeviceMeshShaderFeaturesNV& vulkan::PhysicalDevice::get_mesh_shader_features() const noexcept
{
	return m_meshShaderFeatures;
}

const VkPhysicalDeviceProperties& vulkan::PhysicalDevice::get_properties() const noexcept
{
	return m_properties.properties;
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

const VkPhysicalDeviceMeshShaderPropertiesNV& vulkan::PhysicalDevice::get_mesh_shader_properties() const noexcept
{
	return m_meshShaderProperties;
}

const vulkan::Extensions& vulkan::PhysicalDevice::get_extensions() const noexcept
{
	return m_extensions;
}

std::optional<VkImageFormatProperties> vulkan::PhysicalDevice::get_image_format_properties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const noexcept
{
	VkImageFormatProperties properties;
	auto result = vkGetPhysicalDeviceImageFormatProperties(m_vkHandle, format, type, tiling, usage, flags, &properties);
	if (result == VK_ERROR_FORMAT_NOT_SUPPORTED)
		return std::nullopt;
	else 
		return properties;
}

void vulkan::PhysicalDevice::init_queues() noexcept
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkHandle, &numQueueFamilies, nullptr);
	m_queueFamilyProperties.resize(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(m_vkHandle, &numQueueFamilies, m_queueFamilyProperties.data());

	for (int i = 0; i < m_queueFamilyProperties.size(); i++) {
		const auto& queue = m_queueFamilyProperties[i];
		if ((queue.queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT)) == VK_QUEUE_TRANSFER_BIT) {
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
	m_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	m_features.pNext = &m_11Features;

	m_11Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
	m_11Features.pNext = &m_12Features;

	m_12Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	m_12Features.pNext = &m_13Features;

	m_13Features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
	m_13Features.pNext = &m_accelerationStructureFeatures;

	m_accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	m_accelerationStructureFeatures.pNext = &m_rayTracingPipelineFeatures;

	m_rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	m_rayTracingPipelineFeatures.pNext = &m_vertexInputDynamicStateFeatures;

	m_vertexInputDynamicStateFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
	m_vertexInputDynamicStateFeatures.pNext = &m_meshShaderFeatures;

	m_meshShaderFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_NV;
	m_meshShaderFeatures.pNext = nullptr;


	vkGetPhysicalDeviceFeatures2(m_vkHandle, &m_features);
	m_13Features.pNext = nullptr;
}

void vulkan::PhysicalDevice::init_properties() noexcept
{
	m_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	m_properties.pNext = &m_11Properties;

	m_11Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
	m_11Properties.pNext = &m_12Properties;

	m_12Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
	m_12Properties.pNext = &m_13Properties;

	m_13Properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
	m_13Properties.pNext = &m_accelerationStructureProperties;

	m_accelerationStructureProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR;
	m_accelerationStructureProperties.pNext = &m_rayTracingPipelineProperties;

	m_rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	m_rayTracingPipelineProperties.pNext = &m_meshShaderProperties;

	m_meshShaderProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_NV;
	m_meshShaderProperties.pNext = nullptr;

	vkGetPhysicalDeviceProperties2(m_vkHandle, &m_properties);
}