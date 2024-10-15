module;

#include <cassert>
#include <expected>
#include <utility>
#include <span>

#include "magic_enum_all.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkan;
import NYANLog;
import NYANData;

using namespace nyan::vulkan;


LogicalDevice::LogicalDevice(LogicalDevice&& other) noexcept :
	m_deviceWrapper(std::move(other.m_deviceWrapper)),
	m_physicalDevice(std::move(other.m_physicalDevice)),
	m_allocator(std::move(other.m_allocator)),
	m_deletionQueue(std::move(other.m_deletionQueue)),
	m_queues(std::move(other.m_queues)),
	m_enabledExtensions(std::move(other.m_enabledExtensions))
{
	update_wrapper_pointers();
	
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
		update_wrapper_pointers();
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


	magic_enum::enum_for_each<Queue::Type>([&](auto val) {
		constexpr Queue::Type queueType = val;
		auto queueFamilyIndex = physicalDevice.get_queue_family_index(static_cast<Queue::Type>(queueType));
		if (queuePriorities[queueType].size() > 0) {
			if (queueFamilyIndex != ~0u) {
				queueCreateInfos.push_back(VkDeviceQueueCreateInfo{
					.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					.queueFamilyIndex = queueFamilyIndex,
					.queueCount = static_cast<uint32_t>(queuePriorities[queueType].size()),
					.pQueuePriorities = queuePriorities[queueType].data()
					});
			}
			else
			{
				log::warning().message("[LogicalDevice] Requested queues from non-existent queue families");
			}
		}

		});
	

	assert(!queueCreateInfos.empty());
	if(queueCreateInfos.empty()) [[unlikely]] //Not sure if that is actually invalid but a device without queue is probably never intended
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
	if (const auto result = vkCreateDevice(physicalDevice.get_handle(), &createInfo, nullptr, &deviceHandle); result != VK_SUCCESS) [[unlikely]] {
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

	//auto allocatorResult = Allocator::create(instance, deviceWrapper, physicalDevice, VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT);
	//if(!allocatorResult)
	//	return std::unexpected{ LogicalDeviceCreationError::Type::AllocatorCreationError };
	VmaAllocatorCreateFlags createFlags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
	
	::VmaAllocator allocator;
	if (const auto result = deviceWrapper.vmaCreateAllocator(createFlags, physicalDevice.get_handle(),
	instance.get_handle(), physicalDevice.get_properties().apiVersion, &allocator); result != VK_SUCCESS) [[unlikely]] 
		//return std::unexpected{ Error{result} };
		return std::unexpected{ LogicalDeviceCreationError::Type::AllocatorCreationError };

	//return Allocator{logicalDevice, allocator };

	return LogicalDevice(std::move(deviceWrapper), std::move(physicalDevice), allocator, std::move(enabledExtensions), queuePriorities);
}

const PhysicalDevice& LogicalDevice::get_physical_device() const noexcept
{
	return m_physicalDevice;
}

const LogicalDeviceWrapper& LogicalDevice::get_device() const noexcept
{
	return m_deviceWrapper;
}

DeletionQueue& nyan::vulkan::LogicalDevice::get_deletion_queue() noexcept
{
	return m_deletionQueue;
}

const PhysicalDevice::Extensions& LogicalDevice::get_enabled_extensions() const noexcept
{
	return m_enabledExtensions;
}

std::span<const Queue> LogicalDevice::get_queues(Queue::Type type) const noexcept
{
	return { m_queues[type].data(), m_queues[type].size() };
}

std::span<Queue> LogicalDevice::get_queues(Queue::Type type) noexcept
{
	return { m_queues[type].data(), m_queues[type].size() };
}

Allocator& LogicalDevice::get_allocator() noexcept
{
	return m_allocator;
}

const Allocator& LogicalDevice::get_allocator() const noexcept
{
	return m_allocator;
}

std::expected<void, Error> LogicalDevice::wait_idle() const noexcept
{
	if(auto result = m_deviceWrapper.vkDeviceWaitIdle(); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{result};
	return {};
}

LogicalDevice::LogicalDevice(LogicalDeviceWrapper device, PhysicalDevice physicalDevice, VmaAllocator allocator,
                             PhysicalDevice::Extensions enabledExtensions,
                             const QueueContainer<float>& queuePriorities) noexcept :
	m_deviceWrapper(std::move(device)),
	m_physicalDevice(std::move(physicalDevice)),
	m_allocator(m_deviceWrapper, allocator),
	m_deletionQueue(m_deviceWrapper, m_allocator),
	m_enabledExtensions(std::move(enabledExtensions))
{
	init_queues(queuePriorities);
}

void LogicalDevice::init_queues(const QueueContainer<float>& queuePriorities) noexcept
{
	uint32_t queueCount{ 0 };
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice.get_handle(), &queueCount, nullptr);
	std::vector< VkQueueFamilyProperties> properties(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice.get_handle(), &queueCount, properties.data());

	magic_enum::enum_for_each<Queue::Type>([&](auto val) {
		constexpr Queue::Type queueType = val;
		if (auto queueFamilyIndex = m_physicalDevice.get_queue_family_index(queueType); queueFamilyIndex != ~0u) {
			auto& property = properties[queueFamilyIndex];
			auto& granularity = property.minImageTransferGranularity;
			auto granularityArray = std::array<uint32_t, 3>{ granularity.width, granularity.height, granularity.depth };
			assert(queuePriorities[queueType].size() <= property.queueCount);
			for (uint32_t queueIdx = 0; queueIdx < queuePriorities[queueType].size(); ++queueIdx) {
				VkQueue queueHandle{ VK_NULL_HANDLE };
				m_deviceWrapper.vkGetDeviceQueue(queueFamilyIndex, queueIdx, &queueHandle);

				assert(queueHandle != VK_NULL_HANDLE);
				// we can ignore since emplace shouldn't fail since the sizes are identically due to being the same type
				nyan::ignore = m_queues[queueType].emplace_back(m_deviceWrapper, queueHandle, queueType, Queue::FamilyIndex{ queueFamilyIndex }, queuePriorities[queueType][queueIdx], granularityArray);
			}
		}

		});

}

void LogicalDevice::update_wrapper_pointers() noexcept
{
	m_allocator.ptr_device = &m_deviceWrapper;
	m_deletionQueue.ptr_device = &m_deviceWrapper;
	m_deletionQueue.ptr_allocator = &m_allocator;

	for (auto& queues : m_queues)
		for (auto& queue : queues)
			queue.ptr_device = &m_deviceWrapper;
}
