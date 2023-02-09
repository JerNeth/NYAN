#include "Utility/Exceptions.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/Instance.h"

#include <stdexcept>

[[maybe_unused]] static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback([[maybe_unused]] VkDebugReportFlagsEXT flags,
	[[maybe_unused]] VkDebugReportObjectTypeEXT objectType, [[maybe_unused]] uint64_t object, [[maybe_unused]] size_t location,
	[[maybe_unused]] int32_t messageCode, [[maybe_unused]] const char* pLayerPrefix, [[maybe_unused]] const char* pMessage,[[maybe_unused]] void* pUserData)
{
	Utility::Logger logger{};
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
		logger.message("[Information] ");
	else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
		logger.message("[Warning] ");
	else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		logger.message("[Performance-Warning] ");
	else if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		logger = Utility::log_error();
		logger.message("[Error] ");
	}
	else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
		logger.message("[Debug] ");
	logger.message("\n[");
	switch (objectType) {
		case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT:
			logger.message("Unknown object");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:
			logger.message("Instance");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:
			logger.message("Physical device");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:
			logger.message("Logical device");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:
			logger.message("Queue");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:
			logger.message("Semaphore");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:
			logger.message("Command buffer");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:
			logger.message("Fence");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:
			logger.message("Device Memory");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:
			logger.message("Buffer");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:
			logger.message("Image");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:
			logger.message("Event");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:
			logger.message("Query pool");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:
			logger.message("Buffer view");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:
			logger.message("Image view");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:
			logger.message("Shader module");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:
			logger.message("Pipeline cache");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:
			logger.message("Pipeline layout");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:
			logger.message("Render pass");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:
			logger.message("Pipeline");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:
			logger.message("Descritpor set layout");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:
			logger.message("Sampler");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:
			logger.message("Descriptor pool");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:
			logger.message("Descriptor set");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:
			logger.message("Framebuffer");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:
			logger.message("Command pool");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:
			logger.message("Surface KHR");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:
			logger.message("Swapchain KHR");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_CALLBACK_EXT_EXT:
			logger.message("Debug report callback");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:
			logger.message("Display KHR");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:
			logger.message("Display mode KHR");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT_EXT:
			logger.message("Validation cache EXT");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:
			logger.message("Sampler YCBCR conversion");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:
			logger.message("Descriptor update template");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:
			logger.message("Cu module NVX");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:
			logger.message("Cu function NVX");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:
			logger.message("Acceleration structure KHR");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:
			logger.message("Acceleration structure NV");
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:
			logger.message("Buffer collection fuchsia");
			break;
	}
	logger.message("]");
	Utility::log_error().format("({:#x}[{}]):\n{}n", object, vulkan::VulkanObject<VkInstance>::get_debug_label(object), pMessage);
	return VK_FALSE;
}

vulkan::Instance::Instance(const Validation& validation, const char** extensions, uint32_t extensionCount, std::string applicationName, std::string engineName)
	:
	m_validation(validation),
	m_applicationName(applicationName),
	m_engineName(engineName)
{
	volkInitialize();
	create_instance(extensions, extensionCount);

}

vulkan::Instance::~Instance() {
	if (m_debugReport != VK_NULL_HANDLE)
		vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, m_allocator);
	vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
	vkDestroyInstance(m_instance, m_allocator);

}
std::unique_ptr<vulkan::LogicalDevice> vulkan::Instance::setup_device(const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions)
{
	uint32_t numDevices;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);
	assert(numDevices);
	if (numDevices == 0) {
		throw std::runtime_error("VK: no physical device with Vulkan support available");
	}
	
	std::vector<VkPhysicalDevice> physicalDeviceHandles(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, physicalDeviceHandles.data());
	size_t bestDeviceIdx = 0;
	int bestDeviceOptCount = 0;
	for (size_t i = 0; i < numDevices; i++) {
		m_physicalDevices.emplace_back(physicalDeviceHandles[i]);
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
			bestDeviceIdx = i;
			bestDeviceOptCount = optional;
		}

	}
	m_physicalDevice = m_physicalDevices[bestDeviceIdx].get_handle();

	return m_physicalDevices[bestDeviceIdx].create_logical_device(*this);
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

const std::vector<vulkan::PhysicalDevice>& vulkan::Instance::get_physical_devices() const noexcept
{
	return m_physicalDevices;
}

void vulkan::Instance::create_instance(const char** requestedExtensions, uint32_t requestedExtensionCount, uint32_t applicationVersion, uint32_t engineVersion)
{
	VkApplicationInfo applicationInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = m_applicationName.c_str(),
				.applicationVersion = applicationVersion,
				.pEngineName = m_engineName.c_str(),
				.engineVersion = engineVersion,
				.apiVersion = VK_API_VERSION_1_3
	};

	if(m_validation.enabled)
		m_layers.push_back("VK_LAYER_KHRONOS_validation");

	uint32_t layerPropertyCount;
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
	std::vector<VkLayerProperties> layerProperties(layerPropertyCount);
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties.data());

	for (auto it = m_layers.begin(); it != m_layers.end();) {
		if (std::find_if(layerProperties.begin(), layerProperties.end(), [&it](auto& val) {return std::strcmp(*it, val.layerName) == 0; }) == layerProperties.end()) {
			Utility::log().location().format("Requested instance layer not available: {}", *it);
			it = m_layers.erase(it);
		}
		else
			++it;
	}
	std::vector<const char*> extensions;
	extensions.assign(requestedExtensions, requestedExtensions + requestedExtensionCount);
	{
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, properties.data());
		m_extensions.reserve(propertyCount);
		for (const auto& ext : extensions)
			if (std::find_if(properties.cbegin(), properties.cend(), [&ext](const auto& val) {return std::strcmp(ext, val.extensionName) == 0; }) != properties.cend())
				m_extensions.push_back(ext);	
	}

	for (auto& layer : m_layers) {
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, properties.data());

		m_extensions.reserve(m_extensions.size() + propertyCount);
		for (const auto& ext : extensions)
			if (std::find_if(properties.cbegin(), properties.cend(), [&ext](const auto& val) {return std::strcmp(ext, val.extensionName) == 0; }) != properties.cend() &&
				std::find_if(m_extensions.cbegin(), m_extensions.cend(), [&ext](const auto& val) {return std::strcmp(ext, val) == 0; }) == m_extensions.cend())
				m_extensions.push_back(ext);
	}

	for (const auto& ext : m_extensions)
		if (std::find_if(extensions.cbegin(), extensions.cend(), [&ext](const auto& val) {return std::strcmp(ext, val) == 0; }) == extensions.cend())
			Utility::log().location().format("Requested instance extension not available: {}", ext);
	
	std::array<VkValidationFeatureEnableEXT, 5> validationFeatureEnables{};
	std::array<VkValidationFeatureDisableEXT, 8> validationFeatureDisables{};

	VkValidationFeaturesEXT validationFeatures{
		.sType {VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT},
		.pNext {nullptr},
		.enabledValidationFeatureCount { 0 },
		.pEnabledValidationFeatures { validationFeatureEnables.data() },
		.disabledValidationFeatureCount { 0 },
		.pDisabledValidationFeatures {validationFeatureDisables.data()}
	};
	{
		if (m_validation.gpuAssisted)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
		if (m_validation.gpuAssistedReserveBindingSlot)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT;
		if (m_validation.bestPractices)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
		if (m_validation.debugPrintf)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
		if (m_validation.synchronizationValidation)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
	}

	{
		if (m_validation.disableAll)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT;
		if (m_validation.disableShaders)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT;
		if (m_validation.disableThreadSafety)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT;
		if (m_validation.disableAPIParameters)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT;
		if (m_validation.disableObjectLifetimes)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT;
		if (m_validation.disableCoreChecks)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT;
		if (m_validation.disableUniqueHandles)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT;
		if (m_validation.disableShaderValidationCache)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT;
	}
	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext {nullptr},
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(m_layers.size()),
		.ppEnabledLayerNames = m_layers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(m_extensions.size()),
		.ppEnabledExtensionNames = m_extensions.data()
	};
	if (m_validation.enabled)
		createInfo.pNext = &validationFeatures;
	if (auto result = vkCreateInstance(&createInfo, m_allocator, &m_instance)) {
		throw Utility::VulkanException(result);
	}
	volkLoadInstance(m_instance);
	if (m_validation.createCallback) {
		assert(m_validation.enabled);
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