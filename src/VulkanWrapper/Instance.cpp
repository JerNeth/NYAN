#include "Instance.h"


[[maybe_unused]] static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData)
{
	
	(void)flags; (void)object; (void)location; (void)messageCode; (void)pUserData; (void)pLayerPrefix; // Unused arguments
	fprintf(stderr, "[vulkan] Debug report from ObjectType: %i\nMessage: %s\n\n", objectType, pMessage);
	return VK_FALSE;
}

//vulkan::LogicalDevice vulkan::Instance::setup_device_direct()
//{
//	return *setup_device().release();
//}

std::unique_ptr<vulkan::LogicalDevice> vulkan::Instance::setup_device()
{
	uint32_t numDevices;
	std::vector<VkPhysicalDevice> devices;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);
	if (numDevices == 0) {
		throw std::runtime_error("VK: no physical device with Vulkan support available");
	}
	
	devices.resize(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, devices.data());
	if (auto selectedDevice = std::find_if(devices.cbegin(), devices.cend(),
		[this](const auto& device) {return this->is_device_suitable(device); }); selectedDevice != devices.cend()) {
		m_physicalDevice = *selectedDevice;
		/*VkSurfaceCapabilitiesKHR surfaceCapabilites;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfaceCapabilites);*/
		auto [device, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex] = setup_logical_device(*selectedDevice);
		return std::make_unique<LogicalDevice>(*this, device, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex, *selectedDevice);
		
	}
	else {
		throw std::runtime_error("VK: available devices do not support the required features");
	}
}

#ifdef WIN32
void vulkan::Instance::setup_win32_surface(HWND hwnd, HINSTANCE hinstance) {
	VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hinstance,
		.hwnd = hwnd,
	};
	if (auto result = vkCreateWin32SurfaceKHR(m_instance, &createInfo, m_allocator, &m_surface); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create win32 surface, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create win32 surface, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
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
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create Xlib surface, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create Xlib surface, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}
#endif
uint32_t vulkan::Instance::find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties)const 
{
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memoryProperties);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
			return i;
	}
	throw std::runtime_error("VK: Could not find suitable memory type");
}

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
	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(m_layers.size()),
		.ppEnabledLayerNames = m_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size()),
		.ppEnabledExtensionNames = m_extensions.data()
	};
	if (auto result = vkCreateInstance(&createInfo, m_allocator, &m_instance)) {
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			throw std::runtime_error("VK: could not create instance, missing extension");
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			throw std::runtime_error("VK: could not create instance, initialization failed");
		}
		if (result == VK_ERROR_LAYER_NOT_PRESENT) {
			throw std::runtime_error("VK: could not create instance, layer not present");
		}
		if (result == VK_ERROR_INCOMPATIBLE_DRIVER) {
			throw std::runtime_error("VK: could not create instance, incompatible driver");
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create instance, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create instance, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	volkLoadInstance(m_instance);
	if constexpr (debug) {
		//auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
		if (vkCreateDebugReportCallbackEXT == NULL) {
			throw std::runtime_error("VK: could not find debug report callback ext");
		}
		VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
			.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
			.pfnCallback = debugCallback,
			.pUserData = nullptr
		};
		if (auto result = vkCreateDebugReportCallbackEXT(m_instance, &debugReportCallbackCreateInfo, m_allocator, &m_debugReport); result != VK_SUCCESS) {
			throw std::runtime_error("VK: could not create debug report callback");
		}
	}
}

bool vulkan::Instance::device_supports_extensions(const VkPhysicalDevice& device) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	for (const auto& requiredExtension : m_requiredExtensions) {
		if (std::none_of(availableExtensions.cbegin(), availableExtensions.cend(),
			[&requiredExtension](const auto& availableExtension) {
			return strcmp(availableExtension.extensionName, requiredExtension) == 0;
		}))
			return false;
	}
	return true;
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
	return true;
}


bool vulkan::Instance::is_device_suitable(const VkPhysicalDevice& device) const
{
	return device_supports_extensions(device) && device_swapchain_suitable(device);
}

uint32_t vulkan::Instance::get_graphics_family_queue_index(const VkPhysicalDevice& device) const
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilies.data());
	uint32_t i = 0;
	uint32_t graphicsQueueFamilyIndex = 0;
	auto graphicsQueue = std::find_if(queueFamilies.cbegin(), queueFamilies.cend(),
		[this, &device, &i, &graphicsQueueFamilyIndex](const auto& queueFamily) {
		graphicsQueueFamilyIndex = i++;
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, graphicsQueueFamilyIndex, m_surface, &presentSupport);
		return (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && presentSupport;
	});
	if (graphicsQueue == queueFamilies.cend()) {
		throw std::runtime_error("VK: available device has no graphics queue or no present support or they are not the same");
	}
	return graphicsQueueFamilyIndex;
}
uint32_t vulkan::Instance::get_compute_family_queue_index(const VkPhysicalDevice& device) const
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilies.data());
	uint32_t i = 0;
	uint32_t computeQueueFamily = 0;
	auto transferQueue = std::find_if(queueFamilies.cbegin(), queueFamilies.cend(),
		[this, &device, &i, &computeQueueFamily](const auto& queueFamily) {
		computeQueueFamily = i++;
		return (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT);
	});
	if (transferQueue == queueFamilies.cend()) {
		return ~0u;
	}
	return computeQueueFamily;
}
uint32_t vulkan::Instance::get_transfer_family_queue_index(const VkPhysicalDevice& device) const
{
	uint32_t numQueueFamilies = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(numQueueFamilies);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &numQueueFamilies, queueFamilies.data());
	uint32_t i = 0;
	uint32_t transferQueueFamilyIndex = 0;
	auto transferQueue = std::find_if(queueFamilies.cbegin(), queueFamilies.cend(),
		[this, &device, &i, &transferQueueFamilyIndex](const auto& queueFamily) {
		transferQueueFamilyIndex = i++;
		return (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT  )&& !(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT);
	});
	if (transferQueue == queueFamilies.cend()) {
		return ~0u;
	}
	return transferQueueFamilyIndex;
}

std::tuple<VkDevice, uint32_t, uint32_t, uint32_t> vulkan::Instance::setup_logical_device(const VkPhysicalDevice& device) const
{

	auto graphicsQueueFamilyIndex = get_graphics_family_queue_index(device);
	auto computeQueueFamilyIndex = get_compute_family_queue_index(device);
	auto transferQueueFamilyIndex = get_transfer_family_queue_index(device);
	//uint32_t computeQueueFamilyIndex = ~0u;
	//uint32_t transferQueueFamilyIndex = ~0u;
	float queuePriority = 1.0f;

	std::vector< VkDeviceQueueCreateInfo> queueCreateInfos;
	queueCreateInfos.reserve(3);
	queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
		});
	if (computeQueueFamilyIndex != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = computeQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
			});
	}
	if (transferQueueFamilyIndex != ~0u) {
		queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = transferQueueFamilyIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
			});
	}
	std::vector<void*> allocs;
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = nullptr;
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
	std::vector<const char*> usedExtensions(m_requiredExtensions);
	auto** pNext = &deviceFeatures2.pNext;
	for (auto& extension : extensions) {
		//TODO maybe move some of them to required Extensions or not use them
		if (strcmp(extension.extensionName, VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME) == 0) {
			usedExtensions.push_back(VK_KHR_DESCRIPTOR_UPDATE_TEMPLATE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0) {
			usedExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME) == 0) {
			usedExtensions.push_back(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
		}
		//else if (strcmp(extension.extensionName, VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME) == 0) {
		//	usedExtensions.push_back(VK_EXT_FULL_SCREEN_EXCLUSIVE_EXTENSION_NAME);
		//}
		//else if (strcmp(extension.extensionName, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME) == 0) {
		//	usedExtensions.push_back(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME);
		//}
		else if (strcmp(extension.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
			auto* alloc = malloc(sizeof(VkPhysicalDeviceExtendedDynamicStateFeaturesEXT));
			if (!alloc) {
				throw std::runtime_error("Out of memory");
			}
			allocs.push_back(alloc);

			auto* ext = reinterpret_cast<VkPhysicalDeviceExtendedDynamicStateFeaturesEXT*>(alloc);
			ext->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
			*pNext = alloc;
			ext->pNext = nullptr;
			pNext = &ext->pNext;

			usedExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME) == 0) {
			auto* alloc = malloc(sizeof(VkPhysicalDeviceAccelerationStructureFeaturesKHR));
			if (!alloc) {
				throw std::runtime_error("Out of memory");
			}
			allocs.push_back(alloc);

			auto* ext = reinterpret_cast<VkPhysicalDeviceAccelerationStructureFeaturesKHR*>(alloc);
			ext->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
			*pNext = alloc;
			ext->pNext = nullptr;
			pNext = &ext->pNext;

			usedExtensions.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME) == 0) {
			auto* alloc = malloc(sizeof(VkPhysicalDeviceRayTracingPipelineFeaturesKHR));
			if (!alloc) {
				throw std::runtime_error("Out of memory");
			}
			allocs.push_back(alloc);

			auto* ext = reinterpret_cast<VkPhysicalDeviceRayTracingPipelineFeaturesKHR*>(alloc);
			ext->sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
			*pNext = alloc;
			ext->pNext = nullptr;
			pNext = &ext->pNext;

			usedExtensions.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
		}
		else if (strcmp(extension.extensionName, VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME) == 0) {
			usedExtensions.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
		}
		if constexpr (debug) {
			if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0) {
				usedExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
			}
			//else if (strcmp(extension.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
			//	usedExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
			//}
		}
	}

	vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &deviceFeatures2,
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = static_cast<uint32_t>(m_layers.size()), //Ignored with vulkan 1.1
		.ppEnabledLayerNames = m_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(usedExtensions.size()),
		.ppEnabledExtensionNames = usedExtensions.data(),
		.pEnabledFeatures = NULL,
	};

	//std::vector<std::pair<VkFormat, VkFormatProperties>> formatProperties;
	//for (auto format = VK_FORMAT_UNDEFINED; format != VK_FORMAT_ASTC_12x12_SRGB_BLOCK; format = static_cast<VkFormat>(static_cast<size_t>(format) + 1)) {
	//	VkFormatProperties pFormatProperties;
	//	vkGetPhysicalDeviceFormatProperties(device,format,&pFormatProperties);
	//	if(pFormatProperties.bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT)
	//		formatProperties.push_back({format, pFormatProperties});
	//}
	VkDevice logicalDevice;
	if (auto result = vkCreateDevice(device, &createInfo, nullptr, &logicalDevice); result != VK_SUCCESS) {
		if (result == VK_ERROR_TOO_MANY_OBJECTS) {
			throw std::runtime_error("VK: could not create device, too many objects");
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			throw std::runtime_error("VK: could not create device, initialization failed");
		}
		if (result == VK_ERROR_FEATURE_NOT_PRESENT) {
			throw std::runtime_error("VK: could not create device, feature not present");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not create device, device lost");
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create device, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create device, out of device memory");
		}
		if (result == VK_ERROR_EXTENSION_NOT_PRESENT) {
			throw std::runtime_error("VK: could not create device, extension not present");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	for (auto* alloc : allocs) {
		free(alloc);
	}
	return { logicalDevice, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex };
}
