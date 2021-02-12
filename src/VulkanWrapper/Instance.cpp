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
		
		auto [device, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex] = setup_logical_device(*selectedDevice);
		return std::make_unique<LogicalDevice>(*this, device, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex, *selectedDevice);
		
	}
	else {
		throw std::runtime_error("VK: available devices do not support the required features");
	}
}

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

void vulkan::Instance::create_instance()
{
	VkApplicationInfo applicationInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = m_applicationName.c_str(),
				.applicationVersion = VK_MAKE_VERSION(0, 0, 0),
				.pEngineName = m_engineName.c_str(),
				.engineVersion = VK_MAKE_VERSION(0, 0, 0),
				.apiVersion = VK_API_VERSION_1_2
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
		auto vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT");
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


bool vulkan::Instance::device_supports_features(const VkPhysicalDevice& device) const
{
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	if (m_requiredFeatures) {
		if ((*m_requiredFeatures)[0] != static_cast<bool>(deviceFeatures.alphaToOne))
			return false;
		if ((*m_requiredFeatures)[1] != static_cast<bool>(deviceFeatures.depthBiasClamp))
			return false;
		if ((*m_requiredFeatures)[2] != static_cast<bool>(deviceFeatures.depthBounds))
			return false;
		if ((*m_requiredFeatures)[3] != static_cast<bool>(deviceFeatures.depthClamp))
			return false;
		if ((*m_requiredFeatures)[4] != static_cast<bool>(deviceFeatures.drawIndirectFirstInstance))
			return false;
		if ((*m_requiredFeatures)[5] != static_cast<bool>(deviceFeatures.dualSrcBlend))
			return false;
		if ((*m_requiredFeatures)[6] != static_cast<bool>(deviceFeatures.fillModeNonSolid))
			return false;
		if ((*m_requiredFeatures)[7] != static_cast<bool>(deviceFeatures.fragmentStoresAndAtomics))
			return false;
		if ((*m_requiredFeatures)[8] != static_cast<bool>(deviceFeatures.fullDrawIndexUint32))
			return false;
		if ((*m_requiredFeatures)[9] != static_cast<bool>(deviceFeatures.geometryShader))
			return false;
		if ((*m_requiredFeatures)[10] != static_cast<bool>(deviceFeatures.imageCubeArray))
			return false;
		if ((*m_requiredFeatures)[11] != static_cast<bool>(deviceFeatures.independentBlend))
			return false;
		if ((*m_requiredFeatures)[12] != static_cast<bool>(deviceFeatures.inheritedQueries))
			return false;
		if ((*m_requiredFeatures)[13] != static_cast<bool>(deviceFeatures.largePoints))
			return false;
		if ((*m_requiredFeatures)[14] != static_cast<bool>(deviceFeatures.logicOp))
			return false;
		if ((*m_requiredFeatures)[15] != static_cast<bool>(deviceFeatures.multiDrawIndirect))
			return false;
		if ((*m_requiredFeatures)[16] != static_cast<bool>(deviceFeatures.multiViewport))
			return false;
		if ((*m_requiredFeatures)[17] != static_cast<bool>(deviceFeatures.occlusionQueryPrecise))
			return false;
		if ((*m_requiredFeatures)[18] != static_cast<bool>(deviceFeatures.pipelineStatisticsQuery))
			return false;
		if ((*m_requiredFeatures)[19] != static_cast<bool>(deviceFeatures.robustBufferAccess))
			return false;
		if ((*m_requiredFeatures)[20] != static_cast<bool>(deviceFeatures.samplerAnisotropy))
			return false;
		if ((*m_requiredFeatures)[21] != static_cast<bool>(deviceFeatures.sampleRateShading))
			return false;
		if ((*m_requiredFeatures)[22] != static_cast<bool>(deviceFeatures.shaderClipDistance))
			return false;
		if ((*m_requiredFeatures)[23] != static_cast<bool>(deviceFeatures.shaderCullDistance))
			return false;
		if ((*m_requiredFeatures)[24] != static_cast<bool>(deviceFeatures.shaderFloat64))
			return false;
		if ((*m_requiredFeatures)[25] != static_cast<bool>(deviceFeatures.shaderImageGatherExtended))
			return false;
		if ((*m_requiredFeatures)[26] != static_cast<bool>(deviceFeatures.shaderInt16))
			return false;
		if ((*m_requiredFeatures)[27] != static_cast<bool>(deviceFeatures.shaderInt64))
			return false;
		if ((*m_requiredFeatures)[28] != static_cast<bool>(deviceFeatures.shaderResourceMinLod))
			return false;
		if ((*m_requiredFeatures)[29] != static_cast<bool>(deviceFeatures.shaderResourceResidency))
			return false;
		if ((*m_requiredFeatures)[30] != static_cast<bool>(deviceFeatures.shaderSampledImageArrayDynamicIndexing))
			return false;
		if ((*m_requiredFeatures)[31] != static_cast<bool>(deviceFeatures.shaderStorageBufferArrayDynamicIndexing))
			return false;
		if ((*m_requiredFeatures)[32] != static_cast<bool>(deviceFeatures.shaderStorageImageArrayDynamicIndexing))
			return false;
		if ((*m_requiredFeatures)[33] != static_cast<bool>(deviceFeatures.shaderStorageImageExtendedFormats))
			return false;
		if ((*m_requiredFeatures)[34] != static_cast<bool>(deviceFeatures.shaderStorageImageMultisample))
			return false;
		if ((*m_requiredFeatures)[35] != static_cast<bool>(deviceFeatures.shaderStorageImageReadWithoutFormat))
			return false;
		if ((*m_requiredFeatures)[36] != static_cast<bool>(deviceFeatures.shaderStorageImageWriteWithoutFormat))
			return false;
		if ((*m_requiredFeatures)[37] != static_cast<bool>(deviceFeatures.shaderTessellationAndGeometryPointSize))
			return false;
		if ((*m_requiredFeatures)[38] != static_cast<bool>(deviceFeatures.shaderUniformBufferArrayDynamicIndexing))
			return false;
		if ((*m_requiredFeatures)[39] != static_cast<bool>(deviceFeatures.sparseBinding))
			return false;
		if ((*m_requiredFeatures)[40] != static_cast<bool>(deviceFeatures.sparseResidency16Samples))
			return false;
		if ((*m_requiredFeatures)[41] != static_cast<bool>(deviceFeatures.sparseResidency2Samples))
			return false;
		if ((*m_requiredFeatures)[42] != static_cast<bool>(deviceFeatures.sparseResidency4Samples))
			return false;
		if ((*m_requiredFeatures)[43] != static_cast<bool>(deviceFeatures.sparseResidency8Samples))
			return false;
		if ((*m_requiredFeatures)[44] != static_cast<bool>(deviceFeatures.sparseResidencyAliased))
			return false;
		if ((*m_requiredFeatures)[45] != static_cast<bool>(deviceFeatures.sparseResidencyBuffer))
			return false;
		if ((*m_requiredFeatures)[46] != static_cast<bool>(deviceFeatures.sparseResidencyImage2D))
			return false;
		if ((*m_requiredFeatures)[47] != static_cast<bool>(deviceFeatures.sparseResidencyImage3D))
			return false;
		if ((*m_requiredFeatures)[48] != static_cast<bool>(deviceFeatures.tessellationShader))
			return false;
		if ((*m_requiredFeatures)[49] != static_cast<bool>(deviceFeatures.textureCompressionASTC_LDR))
			return false;
		if ((*m_requiredFeatures)[50] != static_cast<bool>(deviceFeatures.textureCompressionBC))
			return false;
		if ((*m_requiredFeatures)[51] != static_cast<bool>(deviceFeatures.textureCompressionETC2))
			return false;
		if ((*m_requiredFeatures)[52] != static_cast<bool>(deviceFeatures.variableMultisampleRate))
			return false;
		if ((*m_requiredFeatures)[53] != static_cast<bool>(deviceFeatures.vertexPipelineStoresAndAtomics))
			return false;
		if ((*m_requiredFeatures)[54] != static_cast<bool>(deviceFeatures.wideLines))
			return false;
	}
	return true;
}

bool vulkan::Instance::device_supports_extensions(const VkPhysicalDevice& device) const
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	for (const auto& requiredExtension : m_requiredExtensions) {
		if (std::find_if(availableExtensions.cbegin(), availableExtensions.cend(),
			[&requiredExtension](const auto& availableExtension) {
			return strcmp(availableExtension.extensionName, requiredExtension) == 0;
		}) == availableExtensions.cend())
			return false;
	}
	return true;
}

bool vulkan::Instance::device_has_properties(const VkPhysicalDevice& device) const
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
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
	return device_supports_extensions(device) && device_supports_features(device) && device_has_properties(device) && device_swapchain_suitable(device);
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
	VkPhysicalDeviceFeatures2 deviceFeatures2;
	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT dynamicState;
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	dynamicState.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT;
	deviceFeatures2.pNext = &dynamicState;
	dynamicState.pNext = nullptr;

	vkGetPhysicalDeviceFeatures2(device, &deviceFeatures2);
	uint32_t count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
	std::vector<VkExtensionProperties> extensions(count);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &count, extensions.data());
	std::vector<const char*> usedExtensions(m_requiredExtensions);
	for (auto& extension : extensions) {
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
		else if (strcmp(extension.extensionName, VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME) == 0) {
			usedExtensions.push_back(VK_EXT_EXTENDED_DYNAMIC_STATE_EXTENSION_NAME);
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
	return { logicalDevice, graphicsQueueFamilyIndex, computeQueueFamilyIndex, transferQueueFamilyIndex };
}
