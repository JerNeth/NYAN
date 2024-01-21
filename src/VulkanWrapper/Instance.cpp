#include "Instance.h"
#include "Instance.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/Instance.h"
#include "Utility/Exceptions.h"

#include <stdexcept>

static constexpr const char* get_object_string(VkObjectType objectType) {
	switch (objectType) {
	case VK_OBJECT_TYPE_UNKNOWN:
		return "Unknown object";
		break;
	case VK_OBJECT_TYPE_INSTANCE:
		return "Instance";
		break;
	case VK_OBJECT_TYPE_PHYSICAL_DEVICE:
		return "Physical device";
		break;
	case VK_OBJECT_TYPE_DEVICE:
		return "Logical device";
		break;
	case VK_OBJECT_TYPE_QUEUE:
		return "Queue";
		break;
	case VK_OBJECT_TYPE_SEMAPHORE:
		return "Semaphore";
		break;
	case VK_OBJECT_TYPE_COMMAND_BUFFER:
		return "Command buffer";
		break;
	case VK_OBJECT_TYPE_FENCE:
		return "Fence";
		break;
	case VK_OBJECT_TYPE_DEVICE_MEMORY:
		return "Device Memory";
		break;
	case VK_OBJECT_TYPE_BUFFER:
		return "Buffer";
		break;
	case VK_OBJECT_TYPE_IMAGE:
		return "Image";
		break;
	case VK_OBJECT_TYPE_EVENT:
		return "Event";
		break;
	case VK_OBJECT_TYPE_QUERY_POOL:
		return "Query pool";
		break;
	case VK_OBJECT_TYPE_BUFFER_VIEW:
		return "Buffer view";
		break;
	case VK_OBJECT_TYPE_IMAGE_VIEW:
		return "Image view";
		break;
	case VK_OBJECT_TYPE_SHADER_MODULE:
		return "Shader module";
		break;
	case VK_OBJECT_TYPE_PIPELINE_CACHE:
		return "Pipeline cache";
		break;
	case VK_OBJECT_TYPE_PIPELINE_LAYOUT:
		return "Pipeline layout";
		break;
	case VK_OBJECT_TYPE_RENDER_PASS:
		return "Render pass";
		break;
	case VK_OBJECT_TYPE_PIPELINE:
		return "Pipeline";
		break;
	case VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT:
		return "Descriptor set layout";
		break;
	case VK_OBJECT_TYPE_SAMPLER:
		return "Sampler";
		break;
	case VK_OBJECT_TYPE_DESCRIPTOR_POOL:
		return "Descriptor pool";
		break;
	case VK_OBJECT_TYPE_DESCRIPTOR_SET:
		return "Descriptor set";
		break;
	case VK_OBJECT_TYPE_FRAMEBUFFER:
		return "Framebuffer";
		break;
	case VK_OBJECT_TYPE_COMMAND_POOL:
		return "Command pool";
		break;
	case VK_OBJECT_TYPE_SURFACE_KHR:
		return "Surface KHR";
		break;
	case VK_OBJECT_TYPE_SWAPCHAIN_KHR:
		return "Swapchain KHR";
		break;
	case VK_OBJECT_TYPE_DISPLAY_KHR:
		return "Display KHR";
		break;
	case VK_OBJECT_TYPE_DISPLAY_MODE_KHR:
		return "Display mode KHR";
		break;
	case VK_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE:
		return "Descriptor update template";
		break;
	case VK_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR:
		return "Acceleration structure KHR";
		break;
	default:
		return "Unknown";
	}
}

[[maybe_unused]] static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback([[maybe_unused]] VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	[[maybe_unused]] VkDebugUtilsMessageTypeFlagsEXT messageTypes, [[maybe_unused]] const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, [[maybe_unused]] void* pUserData)
{
	auto logger = Utility::Logger::info();

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		logger = Utility::Logger::verbose_message("[Verbose] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		logger = Utility::Logger::info_message("[Info] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		logger = Utility::Logger::warning_message("[Warning] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		logger = Utility::Logger::error_message("[Error] ");

	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		logger.message("[General] ");
	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
		logger.message("[Validation] ");
	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
		logger.message("[Performance] ");
	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT)
		logger.message("[Device Address Binding] ");

	if (pCallbackData->queueLabelCount)
		logger.format("\n {}", pCallbackData->pQueueLabels[0].pLabelName);
	if (pCallbackData->cmdBufLabelCount)
		logger.format("\n {}", pCallbackData->pCmdBufLabels[0].pLabelName);
	if (pCallbackData->objectCount) {
		if (pCallbackData->pObjects[0].pObjectName) {
			logger.format("\n\t [{:#x}] [{}] {}", pCallbackData->pObjects[0].objectHandle, get_object_string(pCallbackData->pObjects[0].objectType), pCallbackData->pObjects[0].pObjectName);
		}
		else {
			logger.format("\n\t [{:#x}] [{}]", pCallbackData->pObjects[0].objectHandle, get_object_string(pCallbackData->pObjects[0].objectType));
		}
	}

	logger.format("\n {}", pCallbackData->pMessageIdName);
	logger.format("\n {}", pCallbackData->pMessage);

	return VK_FALSE;
}

vulkan::Instance::Instance(const Validation& validation, std::span<const char*> requiredExtensions, std::span<const char*> optionalExtensions, std::string applicationName, std::string engineName)
	:
	m_validation(validation),
	m_applicationName(applicationName),
	m_engineName(engineName)
{
	if (const auto result = volkInitialize(); result != VK_SUCCESS)
		throw Utility::VulkanException(result);
	init_layers();
	if (m_validation.enabled)
		use_layer("VK_LAYER_KHRONOS_validation");
	init_extensions();
	create_instance(requiredExtensions, optionalExtensions);
	init_physical_devices();
}

vulkan::Instance::~Instance() noexcept {
	if (m_debugMessenger != VK_NULL_HANDLE)
		vkDestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, m_allocator);
	if(m_surface != VK_NULL_HANDLE)
		vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
	if(m_instance != VK_NULL_HANDLE)
		vkDestroyInstance(m_instance, m_allocator);
}
vulkan::Instance::Instance(Instance&& other) noexcept :
	m_instance (std::move(other.m_instance)),
	m_surface(std::move(other.m_surface)),
	m_bestDeviceIdx(std::move(other.m_bestDeviceIdx)),
	m_physicalDevices(std::move(other.m_physicalDevices)),
	m_allocator(std::move(other.m_allocator)),
	m_debugMessenger(std::move(other.m_debugMessenger)),

	m_validation(std::move(other.m_validation)),
	m_usedExtensions(std::move(other.m_usedExtensions)),
	m_layers(std::move(other.m_layers)),
	m_availableLayers(std::move(other.m_availableLayers)),
	m_availableExtensions(std::move(other.m_availableExtensions)),
	m_extensions(std::move(other.m_extensions)),

	m_applicationName(std::move(other.m_applicationName)),
	m_engineName(std::move(other.m_engineName))
{
	other.m_instance = VK_NULL_HANDLE;
	other.m_surface = VK_NULL_HANDLE;
	other.m_debugMessenger = VK_NULL_HANDLE;
}
vulkan::Instance& vulkan::Instance::operator=(Instance&& other) noexcept
{
	if(this != &other)
	{
		std::swap(m_instance, other.m_instance);
		std::swap(m_surface, other.m_surface);
		m_bestDeviceIdx = other.m_bestDeviceIdx;
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_debugMessenger, other.m_debugMessenger);
		m_physicalDevices = std::move(other.m_physicalDevices);

		m_validation = std::move(other.m_validation);
		m_usedExtensions = std::move(other.m_usedExtensions);
		m_layers = std::move(other.m_layers);
		m_availableLayers = std::move(other.m_availableLayers);
		m_availableExtensions = std::move(other.m_availableExtensions);
		m_extensions = std::move(other.m_extensions);
		m_applicationName = std::move(other.m_applicationName);
		m_engineName = std::move(other.m_engineName);
	}
	return *this;
}
std::unique_ptr<vulkan::LogicalDevice> vulkan::Instance::setup_device(const std::span<const char*>& requiredExtensions, const std::span<const char*>& optionalExtensions)
{
	size_t bestDeviceIdx = 0;
	int bestDeviceOptCount = 0;
	for (size_t i = 0; i < m_physicalDevices.size(); i++) {
		bool required = true;
		int optional = 0;
		auto& dev = m_physicalDevices[i];
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

	float queuePriority = 1.0f;
	auto init_priorities = [](auto n, auto numPriorities)
	{
		std::vector<float> priorities(n);
		for (size_t idx = 0; idx < priorities.size(); ++idx)
			priorities[idx] = 1.f - (idx / Math::max(numPriorities, 1.f));
		return priorities;
	};
	const auto genericQueuePriorities = init_priorities(1, m_physicalDevices[bestDeviceIdx].get_properties().limits.discreteQueuePriorities);
	const auto computeQueuePriorities = init_priorities(1, m_physicalDevices[bestDeviceIdx].get_properties().limits.discreteQueuePriorities);
	const auto transferQueuePriorities = init_priorities(1, m_physicalDevices[bestDeviceIdx].get_properties().limits.discreteQueuePriorities);
	return m_physicalDevices[bestDeviceIdx].create_logical_device(*this);
}

#ifdef WIN32
void vulkan::Instance::setup_win32_surface(HWND hwnd, HINSTANCE hinstance) {
	assert(m_extensions.surface);
	assert(m_extensions.surfaceWin32);
	VkWin32SurfaceCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
		.hinstance = hinstance,
		.hwnd = hwnd,
	};
	if (auto result = vkCreateWin32SurfaceKHR(m_instance, &createInfo, m_allocator, &m_surface); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}
#elif X_PROTOCOL
void vulkan::Instance::setup_x11_surface(Window window, Display* dpy) {
	assert(m_extensions.surface);
	assert(m_extensions.surfaceX11);
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

VkSurfaceKHR vulkan::Instance::get_surface() const
{
	return m_surface;
}

vulkan::Instance::operator VkInstance() const noexcept
{
	return m_instance;
}

std::span<const vulkan::PhysicalDevice> vulkan::Instance::get_physical_devices() const noexcept
{
	return m_physicalDevices;
}

std::span<vulkan::PhysicalDevice> vulkan::Instance::get_physical_devices() noexcept
{
	return m_physicalDevices;
}

void vulkan::Instance::create_instance(const std::span<const char*> requiredExtensions, const  std::span<const char*> optionalExtensions, const  uint32_t applicationVersion, const  uint32_t engineVersion)
{
	if(vkEnumerateInstanceVersion == nullptr)
		throw std::runtime_error("VK: Vulkan 1.0 not supported, update your drivers.");

	uint32_t apiVersion{ VK_API_VERSION_1_0 };
	vkEnumerateInstanceVersion(&apiVersion);

	if(apiVersion < VK_API_VERSION_1_3)
		throw std::runtime_error("VK: Available Vulkan version (<1.3) not supported, update your drivers.");

	VkApplicationInfo applicationInfo{
				.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
				.pApplicationName = m_applicationName.c_str(),
				.applicationVersion = applicationVersion,
				.pEngineName = m_engineName.c_str(),
				.engineVersion = engineVersion,
				.apiVersion = apiVersion
	};
	
	for (const auto& ext : requiredExtensions)
		if(!use_extension(ext))
			throw std::runtime_error("VK: required instance extension missing");

	for (const auto& ext : optionalExtensions)
		use_extension(ext);

	
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
		.enabledExtensionCount = static_cast<uint32_t>(m_usedExtensions.size()),
		.ppEnabledExtensionNames = m_usedExtensions.data()
	};
	if (m_extensions.validationFeatures)
		createInfo.pNext = &validationFeatures;
	if (auto result = vkCreateInstance(&createInfo, m_allocator, &m_instance)) {
		throw Utility::VulkanException(result);
	}
	//volkLoadInstance(m_instance);
	volkLoadInstanceOnly(m_instance);
	if (m_validation.createCallback) {
		assert(m_validation.enabled);
		assert(vkCreateDebugUtilsMessengerEXT);
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{
			.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT},
			.flags {},
			.messageSeverity{static_cast<VkDebugUtilsMessageSeverityFlagsEXT>((m_validation.callBackVerbose ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT  : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (m_validation.callBackInfo ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (m_validation.callBackWarning ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (m_validation.callBackError ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0)))},
			.messageType {static_cast<VkDebugUtilsMessageTypeFlagsEXT>((m_validation.callBackGeneral ? VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (m_validation.callBackValidation ? VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (m_validation.callBackPerformance ? VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (m_validation.callBackDeviceAddressBinding ? VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0)))},
			.pfnUserCallback {debugCallback},
			.pUserData {nullptr}
		};
		if (auto result = vkCreateDebugUtilsMessengerEXT(m_instance, &debugMessengerCreateInfo, m_allocator, &m_debugMessenger); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
	}
}

bool vulkan::Instance::use_extension(const char* extension) noexcept
{
	for (const auto& ext : m_availableExtensions) {
		if (strcmp(ext.extensionName, extension) == 0) {
			if (std::find(std::begin(m_usedExtensions), std::end(m_usedExtensions), extension) == std::end(m_usedExtensions))
				m_usedExtensions.push_back(extension);
			else
				return true;

			if (strcmp(ext.extensionName, VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME) == 0) {
				if (use_extension(VK_KHR_SURFACE_EXTENSION_NAME))
					m_extensions.surfaceCapabilites2 = true;
			}
			else if (strcmp(ext.extensionName, VK_KHR_SURFACE_EXTENSION_NAME) == 0) {
				m_extensions.surface = true;
			}
#ifdef WIN32
			else if (strcmp(ext.extensionName, VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0) {
				if(use_extension(VK_KHR_SURFACE_EXTENSION_NAME))
					m_extensions.surfaceWin32 = true;
			}		
#elif X_PROTOCOL
			else if (strcmp(ext.extensionName, VK_KHR_X11_SURFACE_EXTENSION_NAME) == 0) {
				if (use_extension(VK_KHR_SURFACE_EXTENSION_NAME))
					m_extensions.surfaceX11 = true;
			}
#endif
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_UTILS_EXTENSION_NAME) == 0) {
				m_extensions.debugUtils = true;
			}
			else if (strcmp(ext.extensionName, VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) {
				m_extensions.debugReport = true;
			}
			else if (strcmp(ext.extensionName, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME) == 0) {
				m_extensions.validationFeatures = true;
			}
			else if (strcmp(ext.extensionName, VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME) == 0) {
				if (use_extension(VK_KHR_SURFACE_EXTENSION_NAME))
					m_extensions.swapchainColorSpace = true;
			}
			return true;
		}
	}
	Utility::Logger::info().location().format("Requested instance extension not available: {}", extension);
	return false;
}
bool vulkan::Instance::use_layer(const char* layerName) noexcept
{
	for (const auto& layer : m_availableLayers) {
		if (strcmp(layer.layerName, layerName) == 0) {
			if (std::find(std::begin(m_layers), std::end(m_layers), layerName) == std::end(m_layers))
				m_layers.push_back(layerName);
			return true;
		}
	}
	Utility::Logger::info().location().format("Requested instance layer not available: {}", layerName);
	return false;
}
void vulkan::Instance::init_layers() noexcept
{
	uint32_t layerPropertyCount;
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);
	m_availableLayers.resize(layerPropertyCount);
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, m_availableLayers.data());
}
//VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME //depends on surface
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME //depends on surface
//VK_EXT_DEBUG_UTILS_EXTENSION_NAME
//VK_EXT_DEBUG_REPORT_EXTENSION_NAME //deprecated
//VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME //depends on surface

void vulkan::Instance::init_extensions() noexcept
{
	{
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr);
		m_availableExtensions.resize(propertyCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, m_availableExtensions.data());
	}

	for (auto& layer : m_layers) {
		uint32_t propertyCount;
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, nullptr);
		std::vector<VkExtensionProperties> properties(propertyCount);
		vkEnumerateInstanceExtensionProperties(layer, &propertyCount, properties.data());

		for (const auto& ext : properties)
			if (std::find_if(m_availableExtensions.cbegin(), m_availableExtensions.cend(), [&ext](const auto& val) {return std::strcmp(ext.extensionName, val.extensionName) == 0; }) == m_availableExtensions.cend())
				m_availableExtensions.push_back(ext);
	}
}

void vulkan::Instance::init_physical_devices() noexcept
{
	uint32_t numDevices;
	vkEnumeratePhysicalDevices(m_instance, &numDevices, nullptr);
	if (numDevices == 0) {
		assert(numDevices);
	}

	std::vector<VkPhysicalDevice> physicalDeviceHandles(numDevices);
	vkEnumeratePhysicalDevices(m_instance, &numDevices, physicalDeviceHandles.data());
	for (auto handle : physicalDeviceHandles) {
		auto& dev = m_physicalDevices.emplace_back(handle);
		auto& properties = dev.get_properties(); //VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		std::string deviceType {};
		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_OTHER)
			deviceType = "VK_PHYSICAL_DEVICE_TYPE_OTHER";
		else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
			deviceType = "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
		else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			deviceType = "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
		else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU)
			deviceType = "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
		else if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_CPU)
			deviceType = "VK_PHYSICAL_DEVICE_TYPE_CPU";
		Utility::Logger::info().format("Vulkan capable device: {}\n{}\n{}.{}.{}.{}", properties.deviceName, deviceType, 
			VK_API_VERSION_VARIANT(properties.apiVersion), VK_API_VERSION_MAJOR(properties.apiVersion),
			VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));
	}
}
