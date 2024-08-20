module;

#include <expected>
#include <optional>
#include <memory>
#include <vector>

//#include "volk.h"

module NYANVulkanRenderer;
import NYANVulkanWrapper;

using namespace nyan::vulkan::renderer;
using namespace nyan::vulkan::wrapper;

std::expected<Renderer, RendererCreationError> Renderer::create() noexcept
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
	auto applicationName = " LogicalDeviceTests";
	auto engineName = " LogicalDeviceTests";
	auto instanceCreateResult = Instance::create(validationSettings, requiredExtensions, optionalExtensions, applicationName, engineName);
	if (!instanceCreateResult)
		return std::unexpected{RendererCreationError::Type::InstanceCreationError };

	auto instance = std::move(*instanceCreateResult);
	//instance = std::make_unique<Instance>(std::move(*instanceCreateResult));


	//PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions::vulkan12_core();
	PhysicalDevice::Extensions requiredDeviceExtensions = PhysicalDevice::Extensions{
		.swapchain = 1,
		.bufferDeviceAddress = 1,
		.timelineSemaphore = 1,
		.descriptorIndexing = 1,
	};

	auto physicalDeviceResult = instance.select_physical_device(std::nullopt, requiredDeviceExtensions);
	//if(!physicalDeviceResult)
	//	return std::unexpected{ RendererCreationError::Type:: };

	QueueContainer<float> queuePriorities;
	queuePriorities[Queue::Type::Graphics].push_back(1.f);
	queuePriorities[Queue::Type::Compute].push_back(1.f);
	queuePriorities[Queue::Type::Transfer].push_back(1.f);

	//PhysicalDevice::Extensions extensions{
	//};
	auto logicalDeviceCreateResult = LogicalDevice::create(instance, *physicalDeviceResult,
		nullptr, requiredDeviceExtensions, queuePriorities);
	//if (!logicalDeviceCreateResult)
	//	GTEST_SKIP() << "Could not create LogicalDevice, skipping  Logical Device Tests";
	auto logicalDevice = std::move(*logicalDeviceCreateResult);

	{
		auto& queues = logicalDevice.get_queues(Queue::Type::Graphics);
		queues[0].wait_idle();
	}
	{
		auto& queues = logicalDevice.get_queues(Queue::Type::Compute);
		queues[0].wait_idle();
	}
	{
		auto& queues = logicalDevice.get_queues(Queue::Type::Transfer);
		queues[0].wait_idle();
	}


	return Renderer{};
}