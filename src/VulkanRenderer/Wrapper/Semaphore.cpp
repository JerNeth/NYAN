module;

#include <cassert>
#include <expected>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;
import NYANLog;
import :LogicalDevice;

using namespace nyan::vulkan::wrapper;


Semaphore::Semaphore(Semaphore&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue)
{
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

Semaphore::~Semaphore() noexcept
{
	if (m_handle) {
		r_deletionQueue.queue_semaphore_deletion(m_handle);
	}
}

Semaphore::Semaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue)
{
}

TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& other) noexcept :
	Semaphore(std::move(other)),
	m_timeline(other.m_timeline.exchange(0))
{
}

TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& other) noexcept
{
	if (this != std::addressof(other)) {
		Semaphore::operator=(std::move(other));
		m_timeline = other.m_timeline.exchange(m_timeline.load());
	}
	return *this;
}

std::expected<uint64_t, Error> TimelineSemaphore::get_value() const noexcept
{
	uint64_t value{ 0 };
	if(auto result = r_device.vkGetSemaphoreCounterValue(m_handle, &value); result != VK_SUCCESS)
		return std::unexpected{ result };
	return value;
}

std::expected<TimelineSemaphore, Error> TimelineSemaphore::create(LogicalDevice& device, uint64_t initialValue) noexcept
{
	assert(device.get_enabled_extensions().timelineSemaphore);
	if(!device.get_enabled_extensions().timelineSemaphore)
		return std::unexpected{ VK_ERROR_EXTENSION_NOT_PRESENT };

	VkSemaphoreTypeCreateInfo typeCreateInfo{
		.sType {VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO},
		.pNext {nullptr},
		.semaphoreType {VK_SEMAPHORE_TYPE_TIMELINE },
		.initialValue {initialValue}
	};
	VkSemaphoreCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO},
		.pNext {&typeCreateInfo},
		.flags {0}
	};
	VkSemaphore handle{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateSemaphore(&createInfo, &handle); result != VK_SUCCESS)
		return std::unexpected{result};

	return TimelineSemaphore{ device.get_device(), device.get_deletion_queue(), handle, initialValue };
}

TimelineSemaphore::TimelineSemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t initialValue) noexcept :
	Semaphore(device, deletionQueue, handle),
	m_timeline(initialValue)
{
}

std::expected<BinarySemaphore, Error> BinarySemaphore::create(LogicalDevice& device) noexcept
{
	VkSemaphoreTypeCreateInfo typeCreateInfo{
		.sType {VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO},
		.pNext {nullptr},
		.semaphoreType {VK_SEMAPHORE_TYPE_BINARY},
		.initialValue {0}
	};
	VkSemaphoreCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO},
		.pNext {&typeCreateInfo},
		.flags {0}
	};
	VkSemaphore handle{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateSemaphore(&createInfo, &handle); result != VK_SUCCESS) {
		return std::unexpected{ result };
	}
	return BinarySemaphore{ device.get_device(), device.get_deletion_queue(), handle };
}

BinarySemaphore::BinarySemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept :
	Semaphore(device, deletionQueue, handle)
{
}
