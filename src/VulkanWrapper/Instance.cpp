#include "VkWrapper.h"

void vk::Instance::setup_device() 
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
		auto [device, queueIndex] = setup_logical_device(*selectedDevice);
		m_device = device;
		vkGetDeviceQueue(m_device, queueIndex, 0, &m_graphicsQueue);
	}
	else {
		throw std::runtime_error("VK: available devices do not support the required features");
	}
}

void vk::Instance::setup_win32_surface(HWND hwnd, HINSTANCE hinstance) {
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
	}
}

void vk::Instance::create_instance()
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
		m_layers.push_back("VK_LAYER_LUNARG_standard_validation");
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
	}
}

void vk::Instance::create_image_views()
{
	m_swapChainImageViews.resize(m_swapChainImages.size());
	for (int i = 0; i < m_swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = m_swapChainImages[i],
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = m_swapChainImageFormat,
			.components = {
				.r = VK_COMPONENT_SWIZZLE_IDENTITY,
				.g = VK_COMPONENT_SWIZZLE_IDENTITY, 
				.b = VK_COMPONENT_SWIZZLE_IDENTITY, 
				.a = VK_COMPONENT_SWIZZLE_IDENTITY
			},
			.subresourceRange{
				.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};
		if (auto result = vkCreateImageView(m_device, &createInfo, m_allocator, &m_swapChainImageViews[i]); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not create image view, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not create image view, out of device memory");
			}
		}
	}
}

bool vk::Instance::device_supports_features(const VkPhysicalDevice& device) const
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

bool vk::Instance::device_supports_extensions(const VkPhysicalDevice& device) const
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

bool vk::Instance::device_has_properties(const VkPhysicalDevice& device) const
{
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	return true;
}

bool vk::Instance::device_swapchain_suitable(const VkPhysicalDevice& device) const
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

void vk::Instance::create_swapchain()
{
	VkSurfaceCapabilitiesKHR capabilities;
	
	uint32_t numFormats;
	uint32_t numModes;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numFormats, nullptr);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numModes, nullptr);

	std::vector<VkSurfaceFormatKHR> surfaceFormats(numFormats);
	std::vector<VkPresentModeKHR> presentModes(numModes);
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &numFormats, surfaceFormats.data());
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &numModes, presentModes.data());
	VkSurfaceFormatKHR surfaceFormat = surfaceFormats[0];
	if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(), 
		[](auto format) {return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;});
		_surfaceFormat != surfaceFormats.cend())
		surfaceFormat = *_surfaceFormat;
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.cbegin(), presentModes.cend(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.cend())
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	m_swapChainExtent = capabilities.currentExtent;
	if (m_swapChainExtent.width == UINT32_MAX) {
		m_swapChainExtent = capabilities.maxImageExtent;
	}
	uint32_t imageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
		imageCount = capabilities.maxImageCount;
	}
	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = imageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = m_swapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, //TODO: For defered rendering switch to VK_IMAGE_USAGE_TRANSFER_DST_BIT
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = VK_NULL_HANDLE
	};
	if (auto result = vkCreateSwapchainKHR(m_device, &createInfo, m_allocator, &m_swapChain); result != VK_SUCCESS) {
		if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
			throw std::runtime_error("VK: could not create swapchain, native window in use");
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			throw std::runtime_error("VK: could not create swapchain, initialization failed");
		}
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not create swapchain, surface lost");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not create swapchain, device lost");
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of device memory");
		}
	}
	m_swapChainImageFormat = surfaceFormat.format;
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, nullptr);
	m_swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(m_device, m_swapChain, &imageCount, m_swapChainImages.data());
	create_image_views();
}
void vk::Instance::create_graphics_pipeline()
{
}
bool vk::Instance::is_device_suitable(const VkPhysicalDevice& device) const
{
	return device_supports_extensions(device) && device_supports_features(device) && device_has_properties(device) && device_swapchain_suitable(device);
}

uint32_t vk::Instance::get_graphics_family_queue_index(const VkPhysicalDevice& device) const
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

std::pair<VkDevice, uint32_t> vk::Instance::setup_logical_device(const VkPhysicalDevice& device) const
{

	auto graphicsQueueFamilyIndex = get_graphics_family_queue_index(device);
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = graphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &queueCreateInfo,
		.enabledLayerCount = static_cast<uint32_t>(m_layers.size()), //Ignored with vulkan 1.1
		.ppEnabledLayerNames = m_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(m_requiredExtensions.size()),
		.ppEnabledExtensionNames = m_requiredExtensions.data(),
		.pEnabledFeatures = &deviceFeatures,

	};
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
	}
	return std::make_pair(logicalDevice, static_cast<uint32_t>(graphicsQueueFamilyIndex));
}
