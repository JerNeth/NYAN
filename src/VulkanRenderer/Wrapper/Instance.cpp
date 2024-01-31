module;

#include <cassert>
#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include <array>

#include "volk.h"
module NYANVulkanWrapper;
import NYANLog;

using namespace nyan::vulkan::wrapper;

bool find_layer(std::span<VkLayerProperties> availableLayers, std::string_view layerName)
{
	for (const auto& layer : availableLayers) {
		if (layerName.compare(layer.layerName) == 0) {
			//if (std::find(std::begin(m_layers), std::end(m_layers), layerName) == std::end(m_layers))
			//	m_layers.push_back(layerName);
			return true;
		}
	}
	return false;
}

bool find_extension(std::span<VkExtensionProperties> extensions, std::string_view extensionName)
{
	for (const auto& extension : extensions) {
		if (extensionName.compare(extension.extensionName) == 0) {
			//if (std::find(std::begin(m_layers), std::end(m_layers), layerName) == std::end(m_layers))
			//	m_layers.push_back(layerName);
			return true;
		}
	}
	return false;
}
bool find_extension(std::span<const char*> extensions, std::string_view extensionName)
{
	for (const auto& extension : extensions) {
		if (extensionName.compare(extension) == 0) {
			//if (std::find(std::begin(m_layers), std::end(m_layers), layerName) == std::end(m_layers))
			//	m_layers.push_back(layerName);
			return true;
		}
	}
	return false;
}

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
	auto logger = nyan::util::log::Logger::info();

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
		logger = nyan::util::log::Logger::verbose_message("[Verbose] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		logger = nyan::util::log::Logger::info_message("[Info] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		logger = nyan::util::log::Logger::warning_message("[Warning] ");
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		logger = nyan::util::log::Logger::error_message("[Error] ");

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

const std::vector<PhysicalDevice>& Instance::get_physical_devices() const noexcept
{
	return m_physicalDevices;
}

std::expected<PhysicalDevice, PhysicalDeviceSelectionError> nyan::vulkan::wrapper::Instance::select_physical_device(std::optional<uint64_t> deviceId, const PhysicalDevice::Extensions& requiredExtensions) const noexcept
{

	auto& physicalDevices = get_physical_devices();
	if(physicalDevices.empty())
		return std::unexpected{ PhysicalDeviceSelectionError::Type::NoPhysicalDeviceFoundError };
	std::vector<size_t> validDevices;
	size_t deviceIdx = 0;
	auto bestType = PhysicalDevice::Type::Other;

	for (size_t idx = 0; idx < physicalDevices.size(); ++idx) {
		auto& dev = physicalDevices[idx];

		auto& ext = dev.get_available_extensions();

		if ((ext & requiredExtensions) != requiredExtensions)
			continue;


		//Prefer to run on dedicated but give the option to the user to select
		if (auto devType = dev.get_type(); static_cast<size_t>(bestType) > static_cast<size_t>(devType) && !deviceId)
		{
			bestType = devType;
			deviceIdx = validDevices.size();
		}
		if (deviceId)
			if (*deviceId == dev.get_id())
				deviceIdx = validDevices.size();

		validDevices.push_back(idx);
	}

	if (validDevices.empty())
		return std::unexpected{ PhysicalDeviceSelectionError::Type::NoValidPhysicalDeviceError };

	return physicalDevices[validDevices[deviceIdx]];
}

std::expected<std::vector<PhysicalDevice>, PhysicalDeviceCreationError> enumerate_physical_devices(const VkInstance handle) noexcept
{

	assert(vkEnumeratePhysicalDevices);

	uint32_t numDevices;
	if (const auto result = vkEnumeratePhysicalDevices(handle, &numDevices, nullptr); result != VK_SUCCESS) {
		return std::unexpected{ PhysicalDeviceCreationError::Type::UnknownError };
	}

	std::vector<VkPhysicalDevice> physicalDeviceHandles(numDevices);

	if (const auto result = vkEnumeratePhysicalDevices(handle, &numDevices, physicalDeviceHandles.data()); result != VK_SUCCESS) {
		return std::unexpected{ PhysicalDeviceCreationError::Type::UnknownError };
	}

	std::vector<PhysicalDevice> physicalDevices;
	for (auto handle : physicalDeviceHandles) {
		auto result = PhysicalDevice::create(handle);
		if (!result)
		{
			return std::unexpected{ result.error()};
		}
		auto& dev = physicalDevices.emplace_back(std::move(*result));
		const auto& properties = dev.get_properties(); //VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
		std::string deviceType{};
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
		nyan::util::log::Logger::info().format("[Instance] Found vulkan capable device: {}\n{}\n{}.{}.{}.{}", properties.deviceName, deviceType,
		                                       VK_API_VERSION_VARIANT(properties.apiVersion), VK_API_VERSION_MAJOR(properties.apiVersion),
		                                       VK_API_VERSION_MINOR(properties.apiVersion), VK_API_VERSION_PATCH(properties.apiVersion));
	}
	return physicalDevices;
}

std::expected<Instance, InstanceCreationError> Instance::create(
	const ValidationSettings& validationSettings,
	const ExtensionSettings& requiredExtensions, const ExtensionSettings& optionalExtension,
	const std::string_view applicationName, const std::string_view engineName,
	const uint32_t applicationVersion, const uint32_t engineVersion) noexcept
{
	if (const auto result = volkInitialize(); result != VK_SUCCESS)
		return std::unexpected{InstanceCreationError::Type::VolkInitializationError };
	assert(vkCreateInstance);
	assert(vkEnumerateInstanceVersion);
	assert(vkEnumerateInstanceExtensionProperties);
	assert(vkEnumerateInstanceLayerProperties);

	uint32_t layerPropertyCount {0};
	if (auto result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr); result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
		return std::unexpected{ InstanceCreationError::Type::UnknownError };
	}
	std::vector< VkLayerProperties> availableLayers (layerPropertyCount);
	if (auto result = vkEnumerateInstanceLayerProperties(&layerPropertyCount, availableLayers.data()); result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
		return std::unexpected{ InstanceCreationError::Type::UnknownError };
	}

	std::vector<const char*> requestedLayers;
	if (const auto validationLayer = "VK_LAYER_KHRONOS_validation"; 
		validationSettings.enabled && find_layer(availableLayers, validationLayer))
		requestedLayers.push_back(validationLayer);
	//util::log::Logger::info().location().format("Requested instance layer not available: {}", layerName);

	
	uint32_t propertyCount;
	if(auto result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, nullptr); result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
		return std::unexpected{ InstanceCreationError::Type::UnknownError };
	}
	std::vector<VkExtensionProperties> availableExtensions(propertyCount);
	if (auto result = vkEnumerateInstanceExtensionProperties(nullptr, &propertyCount, availableExtensions.data()); result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
		return std::unexpected{ InstanceCreationError::Type::UnknownError };
	}

	for (const auto& layer : requestedLayers) {
		if (auto result = vkEnumerateInstanceExtensionProperties(layer, &propertyCount, nullptr); result != VK_SUCCESS)
		{
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
				result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
			return std::unexpected{ InstanceCreationError::Type::UnknownError };
		}
		std::vector<VkExtensionProperties> properties(propertyCount);
		if (auto result = vkEnumerateInstanceExtensionProperties(layer, &propertyCount, properties.data()); result != VK_SUCCESS)
		{
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
				result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
			return std::unexpected{ InstanceCreationError::Type::UnknownError };
		}

		for (const auto& ext : properties)
			if (std::find_if(availableExtensions.cbegin(), availableExtensions.cend(), 
				[&ext](const auto& val) {return std::strcmp(ext.extensionName, val.extensionName) == 0; }) == availableExtensions.cend())
				availableExtensions.push_back(ext);
	}

	std::vector<const char*> requestedExtensions;
	
	auto check_extension = [&](auto extensionName)
		{
			if (find_extension(availableExtensions, extensionName)) {
				if(!find_extension(requestedExtensions, extensionName))
					requestedExtensions.push_back(extensionName);
				return true;
			}
			util::log::Logger::info().location().format("Extension: \"{}\" not available", extensionName);
			return false;
		};

	auto add_extensions = [&](auto extensions)
		{
			return ((extensions.surface && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#ifdef VK_USE_PLATFORM_WIN32_KHR
			(extensions.win32Surface && !check_extension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
			(extensions.xlibSurface && !check_extension(VK_KHR_XLIB_SURFACE_EXTENSION_NAME) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
#ifdef VK_USE_PLATFORM_XCB_KHR
			(extensions.xcbSurface && !check_extension(VK_KHR_XCB_SURFACE_EXTENSION_NAME) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
#ifdef VK_USE_PLATFORM_WAYLAND_KHR
			(extensions.waylandSurface && !check_extension(VK_USE_PLATFORM_WAYLAND_KHR) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
#ifdef VK_USE_PLATFORM_ANDROID_KHR
			(extensions.androidSurface && !check_extension(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
			(extensions.acquireXlibSurface && !check_extension(VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME) && (extensions.surface = 1) && !check_extension(VK_KHR_SURFACE_EXTENSION_NAME)) ||
#endif
				(extensions.directModeDisplay && !check_extension(VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME)) ||
				(extensions.display && !check_extension(VK_KHR_DISPLAY_EXTENSION_NAME)) ||
				(extensions.validationFeatures && !check_extension(VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME)) ||
				(extensions.debugUtils && !check_extension(VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) ||
				(extensions.getSurfaceCapabilities2 && !check_extension(VK_KHR_GET_SURFACE_CAPABILITIES_2_EXTENSION_NAME)) ||
				(extensions.surfaceMaintenance1 && !check_extension(VK_EXT_SURFACE_MAINTENANCE_1_EXTENSION_NAME)) ||
				(extensions.getPhysicalDeviceProperties2 && !check_extension(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) ||
				(extensions.swapchainColorSpaceExtension && !check_extension(VK_EXT_SWAPCHAIN_COLOR_SPACE_EXTENSION_NAME)) ||
				(extensions.getDisplayProperties2 && !check_extension(VK_KHR_GET_DISPLAY_PROPERTIES_2_EXTENSION_NAME) && (extensions.display = 1) && !check_extension(VK_KHR_DISPLAY_EXTENSION_NAME)));
		};

	if (add_extensions(requiredExtensions))
		return std::unexpected{ InstanceCreationError::Type::RequiredExtensionNotPresentError };
	if (add_extensions(optionalExtension))
		util::log::Logger::warning_message("[Instance] Requested optional instance extensions not all available");

	if (vkEnumerateInstanceVersion == nullptr)
		return std::unexpected{ InstanceCreationError::Type::APIVersionNotSupportedError };

	uint32_t apiVersion{ VK_API_VERSION_1_0 };
	if(const auto result = vkEnumerateInstanceVersion(&apiVersion); result != VK_SUCCESS)
		return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };

	if (apiVersion < VK_API_VERSION_1_3)
		return std::unexpected{ InstanceCreationError::Type::APIVersionNotSupportedError };

	VkApplicationInfo applicationInfo{
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = applicationName.data(),
		.applicationVersion = applicationVersion,
		.pEngineName = engineName.data(),
		.engineVersion = engineVersion,
		.apiVersion = apiVersion
	};

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
		if (validationSettings.gpuAssisted)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT;
		if (validationSettings.gpuAssistedReserveBindingSlot)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT;
		if (validationSettings.bestPractices)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT;
		if (validationSettings.debugPrintf)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT;
		if (validationSettings.synchronizationValidation)
			validationFeatureEnables[validationFeatures.enabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT;
	}

	{
		if (validationSettings.disableAll)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_ALL_EXT;
		if (validationSettings.disableShaders)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_SHADERS_EXT;
		if (validationSettings.disableThreadSafety)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_THREAD_SAFETY_EXT;
		if (validationSettings.disableAPIParameters)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_API_PARAMETERS_EXT;
		if (validationSettings.disableObjectLifetimes)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_OBJECT_LIFETIMES_EXT;
		if (validationSettings.disableCoreChecks)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_CORE_CHECKS_EXT;
		if (validationSettings.disableUniqueHandles)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_UNIQUE_HANDLES_EXT;
		if (validationSettings.disableShaderValidationCache)
			validationFeatureDisables[validationFeatures.disabledValidationFeatureCount++] = VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT;
	}

	VkInstanceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pNext {nullptr},
		.pApplicationInfo = &applicationInfo,
		.enabledLayerCount = static_cast<uint32_t>(requestedLayers.size()),
		.ppEnabledLayerNames = requestedLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requestedExtensions.size()),
		.ppEnabledExtensionNames = requestedExtensions.data()
	};
	if (find_extension(requestedExtensions, VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME))
		createInfo.pNext = &validationFeatures;

	VkAllocationCallbacks* allocator {nullptr};
	VkInstance handle{ VK_NULL_HANDLE };

	if(auto result = vkCreateInstance(&createInfo, allocator, &handle); result != VK_SUCCESS)
	{
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
		else if (result == VK_ERROR_INCOMPATIBLE_DRIVER)
			return std::unexpected{ InstanceCreationError::Type::IncompatibleDriverError };
		else
			return std::unexpected{ InstanceCreationError::Type::UnknownError };
	}
	volkLoadInstanceOnly(handle);


	VkDebugUtilsMessengerEXT debugUtilsMessenger{ VK_NULL_HANDLE };
	if (validationSettings.createCallback && validationSettings.enabled && find_extension(requestedExtensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME)) {
		assert(vkCreateDebugUtilsMessengerEXT);
		VkDebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo{
			.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT},
			.flags {},
			.messageSeverity{static_cast<VkDebugUtilsMessageSeverityFlagsEXT>((validationSettings.callBackVerbose ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (validationSettings.callBackInfo ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (validationSettings.callBackWarning ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0))
							| (validationSettings.callBackError ? VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT : static_cast<VkDebugUtilsMessageSeverityFlagBitsEXT>(0)))},
			.messageType {static_cast<VkDebugUtilsMessageTypeFlagsEXT>((validationSettings.callBackGeneral ? VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (validationSettings.callBackValidation ? VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (validationSettings.callBackPerformance ? VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0))
							| (validationSettings.callBackDeviceAddressBinding ? VK_DEBUG_UTILS_MESSAGE_TYPE_DEVICE_ADDRESS_BINDING_BIT_EXT : static_cast<VkDebugUtilsMessageTypeFlagBitsEXT>(0)))},
			.pfnUserCallback {debugCallback},
			.pUserData {nullptr}
		};
		if (auto result = vkCreateDebugUtilsMessengerEXT(handle, &debugMessengerCreateInfo, allocator, &debugUtilsMessenger); result != VK_SUCCESS) {
			if(result == VK_ERROR_OUT_OF_HOST_MEMORY)
				return std::unexpected{ InstanceCreationError::Type::OutOfMemoryError };
			return std::unexpected{ InstanceCreationError::Type::UnknownError };
		}
	}
	auto physicalDeviceEnumerationResult = enumerate_physical_devices(handle);
	if (!physicalDeviceEnumerationResult)
		return std::unexpected{ InstanceCreationError::Type::PhysicalDeviceEnumerationError };


	return Instance{ handle, debugUtilsMessenger, allocator, *physicalDeviceEnumerationResult, validationSettings, requiredExtensions };
}

Instance::Instance(Instance&& other) noexcept :
	m_handle(std::move(other.m_handle)),
	m_debugUtilsMessenger(std::move(other.m_debugUtilsMessenger)),
	m_allocator(std::move(other.m_allocator)),
	m_physicalDevices(std::move(other.m_physicalDevices)),
	m_validationSettings(std::move(other.m_validationSettings)),
	m_enabledExtensions(std::move(other.m_enabledExtensions))
{
	other.m_handle = VK_NULL_HANDLE;
	other.m_debugUtilsMessenger = VK_NULL_HANDLE;
}

Instance&  Instance::operator=(Instance&& other)  noexcept
{
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
		std::swap(m_debugUtilsMessenger, other.m_debugUtilsMessenger);
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_physicalDevices, other.m_physicalDevices);
		std::swap(m_validationSettings, other.m_validationSettings);
		std::swap(m_enabledExtensions, other.m_enabledExtensions);
	}
	return *this;
}


VkInstance Instance::get_handle() const noexcept
{
	return m_handle;
}

Instance::Instance(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessenger, VkAllocationCallbacks* allocator,
	std::vector<PhysicalDevice> physicalDevices, const ValidationSettings& validationSettings, ExtensionSettings enabledExtensions) noexcept :
	m_handle(instance),
	m_debugUtilsMessenger(debugUtilsMessenger),
	m_allocator(allocator),
	m_physicalDevices(physicalDevices),
	m_validationSettings(validationSettings),
	m_enabledExtensions(enabledExtensions)
{

}

Instance::~Instance()
{
	assert(vkDestroyInstance);

	if (m_debugUtilsMessenger != VK_NULL_HANDLE && vkDestroyDebugUtilsMessengerEXT)
		vkDestroyDebugUtilsMessengerEXT(m_handle, m_debugUtilsMessenger, m_allocator);
	if (m_handle != VK_NULL_HANDLE && vkDestroyInstance)
		vkDestroyInstance(m_handle, m_allocator);
}
