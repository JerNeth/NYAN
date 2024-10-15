module;

#include <atomic>
#include <cassert>
#include <expected>
#include <utility>

#include "volk.h"

module NYANVulkan;
import NYANLog;
import :LogicalDevice;

using namespace nyan::vulkan;


Semaphore::Semaphore(Semaphore&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	m_timeline(other.m_timeline.exchange(0))
{
}

Semaphore& Semaphore::operator=(Semaphore&& other) noexcept
{
	if (this != std::addressof(other)) {
		std::swap(ptr_device, other.ptr_device);
		std::swap(m_handle, other.m_handle);
		m_timeline = other.m_timeline.exchange(m_timeline.load());
	}
	return *this;
}

Semaphore::~Semaphore() noexcept
{
	if (m_handle) {
		r_deletionQueue.queue_deletion(m_handle);
	}
}

Semaphore::Semaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t timeline) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	m_timeline(timeline)
{
}

TimelineSemaphore::TimelineSemaphore(TimelineSemaphore&& other) noexcept :
	Semaphore(std::move(other))
{
}

TimelineSemaphore& TimelineSemaphore::operator=(TimelineSemaphore&& other) noexcept
{
	if (this != std::addressof(other)) [[unlikely]] {
		Semaphore::operator=(std::move(other));
	}
	return *this;
}

std::expected<uint64_t, Error> TimelineSemaphore::get_value() const noexcept
{
	uint64_t value{ 0 };
	if(auto result = ptr_device->vkGetSemaphoreCounterValue(m_handle, &value); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	return value;
}

std::expected<TimelineSemaphore, Error> TimelineSemaphore::create(LogicalDevice& device, uint64_t initialValue) noexcept
{
	assert(device.get_enabled_extensions().timelineSemaphore);
	if(!device.get_enabled_extensions().timelineSemaphore) [[unlikely]]
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
	if (auto result = device.get_device().vkCreateSemaphore(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{result};

	return TimelineSemaphore{ device.get_device(), device.get_deletion_queue(), handle, initialValue };
}

TimelineSemaphore::TimelineSemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t initialValue) noexcept :
	Semaphore(device, deletionQueue, handle, initialValue)
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
		.pNext { (device.get_physical_device().get_version() < version12) ? nullptr : &typeCreateInfo},
		.flags {0}
	};
	VkSemaphore handle{ VK_NULL_HANDLE };

	if (auto result = device.get_device().vkCreateSemaphore(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return BinarySemaphore{ device.get_device(), device.get_deletion_queue(), handle };
}

bool BinarySemaphore::signaled() const noexcept
{
	return (m_timeline & 0x1) == 1;
}

bool BinarySemaphore::waited() const noexcept
{
	return (m_timeline & 0x1) == 0;
}

BinarySemaphore::BinarySemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept :
	Semaphore(device, deletionQueue, handle, 0)
{
}
