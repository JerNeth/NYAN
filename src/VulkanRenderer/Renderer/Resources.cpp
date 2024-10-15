module;

#include <expected>
#include <optional>
#include <memory>
#include <string_view>
#include <vector>

//#include "volk.h"

module NYANRenderer;
import NYANVulkan;
import NYANData;

using namespace nyan::renderer;
using namespace nyan::vulkan;

std::expected<Resources, ResourceError> Resources::create(Features features, std::string_view applicationName) noexcept
{

	Instance::ValidationSettings validationSettings{
		.enabled {true},
		.createCallback {true},
		.callBackVerbose { true },
		.callBackInfo { true },
		.callBackWarning { true },
		.callBackError { true },
		.callBackGeneral { true },
		.callBackValidation { true },
		.callBackPerformance { true },
		.callBackDeviceAddressBinding { true },
		.gpuAssisted { false },
		.gpuAssistedReserveBindingSlot { false },
		.bestPractices { false },
		.debugPrintf { false },
		.synchronizationValidation { false },
		.disableAll { false },
		.disableShaders { false },
		.disableThreadSafety { false },
		.disableAPIParameters { false },
		.disableObjectLifetimes { false },
		.disableCoreChecks { false },
		.disableUniqueHandles { false },
		.disableShaderValidationCache { false },
	};
	Instance::ExtensionSettings requiredExtensions
	{
		.validationFeatures = 1,
		.debugUtils = 1,
	};
	Instance::ExtensionSettings optionalExtensions{};
	auto engineName = "NYAN";
	auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
	if (!instanceCreateResult) [[unlikely]]
		return std::unexpected{ ResourceError::Type::InstanceCreationError };

	//instance = std::make_unique<Instance>(std::move(*instanceCreateResult));


	//PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions::vulkan12_core();
	PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions{
		.swapchain = 1,
		.bufferDeviceAddress = 1,
		.timelineSemaphore = 1,
		.descriptorIndexing = 1,
	};

	auto physicalDeviceResult = instanceCreateResult->select_physical_device(std::nullopt, requiredDeviceExtensions, version12);
	if(!physicalDeviceResult) [[unlikely]]
		return std::unexpected{ ResourceError::Type::NoValidDevices };

	QueueContainer<float> queuePriorities;
	nyan::ignore = queuePriorities[Queue::Type::Graphics].push_back(1.f);
	nyan::ignore = queuePriorities[Queue::Type::Compute].push_back(1.f);
	nyan::ignore = queuePriorities[Queue::Type::Transfer].push_back(1.f);

	//PhysicalDevice::Extensions extensions{
	//};
	auto logicalDeviceCreateResult = LogicalDevice::create(*instanceCreateResult, *physicalDeviceResult,
		nullptr, requiredDeviceExtensions, queuePriorities);

	if (!logicalDeviceCreateResult) [[unlikely]]
		return std::unexpected{ ResourceError::Type::LogicalDeviceCreationError };


	return Resources{ Data{std::move(*instanceCreateResult), std::move(*logicalDeviceCreateResult)} };
}