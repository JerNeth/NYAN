module;

#include <cassert>
#include <utility>

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkanWrapper;
import NYANLog;

using namespace nyan::vulkan::wrapper;


LogicalDevice::LogicalDevice(LogicalDevice&& other) noexcept :
	m_deviceWrapper(std::move(other.m_deviceWrapper)),
	m_physicalDevice(std::move(other.m_physicalDevice)),
	m_allocator(std::move(other.m_allocator)),
	m_deletionQueue(std::move(other.m_deletionQueue)),
	m_queues(std::move(other.m_queues)),
	m_enabledExtensions(std::move(other.m_enabledExtensions))
{
}

LogicalDevice& LogicalDevice::operator=(LogicalDevice&& other) noexcept
{
	if (this != std::addressof(other))
	{
		std::swap(m_deviceWrapper, other.m_deviceWrapper);
		std::swap(m_physicalDevice, other.m_physicalDevice);
		std::swap(m_allocator, other.m_allocator);
		std::swap(m_deletionQueue, other.m_deletionQueue);
		std::swap(m_queues, other.m_queues);
		std::swap(m_enabledExtensions, other.m_enabledExtensions);
	}
	return *this;
}

LogicalDevice::~LogicalDevice() noexcept
{
}

std::expected<LogicalDevice, LogicalDeviceCreationError> LogicalDevice::create(Instance& instance, PhysicalDevice physicalDevice, const VkAllocationCallbacks* allocatorCallbacks,
	PhysicalDevice::Extensions enabledExtensions,
	const QueueContainer<float>& queuePriorities) noexcept
{
	auto apiVersion = physicalDevice.get_properties().apiVersion;
	auto extensionList = enabledExtensions.generate_extension_list(apiVersion);

	std::vector< VkDeviceQueueCreateInfo> queueCreateInfos;

	for(size_t queueType = 0; queueType < static_cast<size_t>(Queue::Type::Size); ++queueType)
	{
		auto queueFamilyIndex = physicalDevice.get_queue_family_index(static_cast<Queue::Type>(queueType));
		if (!queuePriorities[queueType].empty()) {
			if (queueFamilyIndex != ~0u) {
				queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = queueFamilyIndex,
					.queueCount = static_cast<uint32_t>(queuePriorities[queueType].size()),
					.pQueuePriorities = queuePriorities[queueType].data()
					});
			} else
			{
				util::log::warning_message("[LogicalDevice] Requested queues from non-existent queue families");
			}
		}
	};
	

	assert(!queueCreateInfos.empty());
	if(queueCreateInfos.empty()) //Not sure if that is actually invalid but a device without queue is probably never intended
		return std::unexpected{ LogicalDeviceCreationError::Type::NoQueueSpecified }; 

	VkDeviceCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.pNext = &physicalDevice.build_feature_chain(enabledExtensions),
		.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
		.pQueueCreateInfos = queueCreateInfos.data(),
		.enabledLayerCount = 0,
		.ppEnabledLayerNames = nullptr,
		.enabledExtensionCount = static_cast<uint32_t>(extensionList.size()),
		.ppEnabledExtensionNames = extensionList.data(),
		.pEnabledFeatures = nullptr,
	};

	if (apiVersion < VK_API_VERSION_1_1)
	{
		createInfo.pNext = nullptr;
		createInfo.pEnabledFeatures = &physicalDevice.get_used_features();
	}

	VkDevice deviceHandle;
	if (const auto result = vkCreateDevice(physicalDevice.get_handle(), &createInfo, nullptr, &deviceHandle); result != VK_SUCCESS) {
		if (result == VK_ERROR_DEVICE_LOST) {
			return std::unexpected{ LogicalDeviceCreationError::Type::DeviceLostError };
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY ||
			result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			return std::unexpected{ LogicalDeviceCreationError::Type::OutOfMemoryError };
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			return std::unexpected{ LogicalDeviceCreationError::Type::InitializationFailed };
		}
		if (result == VK_ERROR_TOO_MANY_OBJECTS) {
			return std::unexpected{ LogicalDeviceCreationError::Type::TooManyObjects };
		}
		//We checked beforehand for these cases, otherwise this should indicate program miss function.
		assert(result != VK_ERROR_EXTENSION_NOT_PRESENT);
		assert(result != VK_ERROR_FEATURE_NOT_PRESENT);
		return std::unexpected{ LogicalDeviceCreationError::Type::UnknownError };
	}
	LogicalDeviceWrapper deviceWrapper{ deviceHandle, allocatorCallbacks };

	auto allocatorResult = Allocator::create(instance, deviceWrapper, physicalDevice, VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT);
	if(!allocatorResult)
		return std::unexpected{ LogicalDeviceCreationError::Type::AllocatorCreationError };

	return LogicalDevice(std::move(deviceWrapper), std::move(physicalDevice), std::move(*allocatorResult), std::move(enabledExtensions), queuePriorities);
}

const PhysicalDevice& LogicalDevice::get_physical_device() const noexcept
{
	return m_physicalDevice;
}

const LogicalDeviceWrapper& LogicalDevice::get_device() const noexcept
{
	return m_deviceWrapper;
}

DeletionQueue& nyan::vulkan::wrapper::LogicalDevice::get_deletion_queue() noexcept
{
	return m_deletionQueue;
}

const PhysicalDevice::Extensions& LogicalDevice::get_enabled_extensions() const noexcept
{
	return m_enabledExtensions;
}

const std::vector<Queue>& LogicalDevice::get_queues(Queue::Type type) const noexcept
{
	const auto typeVal = static_cast<size_t>(type);
	assert(typeVal < static_cast<size_t>(Queue::Type::Size));
	return m_queues[typeVal];
}

LogicalDevice::LogicalDevice(LogicalDeviceWrapper device, PhysicalDevice physicalDevice, Allocator allocator,
                             PhysicalDevice::Extensions enabledExtensions,
                             const QueueContainer<float>& queuePriorities) noexcept :
	m_deviceWrapper(std::move(device)),
	m_physicalDevice(std::move(physicalDevice)),
	m_allocator(std::move(allocator)),
	m_deletionQueue(m_deviceWrapper, m_allocator),
	m_enabledExtensions(std::move(enabledExtensions))
{
	init_queues(queuePriorities);
}

void LogicalDevice::init_queues(const QueueContainer<float>& queuePriorities) noexcept
{
	for (size_t queueType = 0; queueType < static_cast<size_t>(Queue::Type::Size); ++queueType)
	{
		if (auto queueFamilyIndex = m_physicalDevice.get_queue_family_index(static_cast<Queue::Type>(queueType)); queueFamilyIndex != ~0u) {
			for (uint32_t queueIdx = 0; queueIdx < queuePriorities[queueType].size(); ++queueIdx) {
				VkQueue queueHandle{ VK_NULL_HANDLE };
				m_deviceWrapper.vkGetDeviceQueue(queueFamilyIndex, queueIdx, &queueHandle);
				assert(queueHandle != VK_NULL_HANDLE);
				m_queues[queueType].emplace_back(m_deviceWrapper, queueHandle, static_cast<Queue::Type>(queueType), queueFamilyIndex, queuePriorities[queueType][queueIdx]);
			}
		}
	}

}
