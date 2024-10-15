module;

#include <cassert>
#include <chrono>
#include <expected>
#include <utility>

#include "volk.h"

module NYANVulkan;
import NYANLog;

using namespace nyan::vulkan;


Fence::Fence(Fence&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	m_signaled(std::exchange(other.m_signaled, false))
{
}

Fence& Fence::operator=(Fence&& other) noexcept
{
	assert(ptr_device == other.ptr_device);
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
		std::swap(m_signaled, other.m_signaled);
	}
	return *this;
}

Fence::~Fence() noexcept
{
	if (m_handle)
		ptr_device->vkDestroyFence(m_handle);
}

bool Fence::is_signaled() const noexcept
{
	return m_signaled;
}

void nyan::vulkan::Fence::set_signaled() noexcept
{
	m_signaled = true;
}

std::expected<bool, Error> Fence::wait(std::chrono::duration<uint64_t, std::nano> timeout) const noexcept
{
	if (auto result = ptr_device->vkWaitForFences(1, &m_handle, VK_TRUE, timeout.count()); result != VK_SUCCESS ||
		result != VK_NOT_READY) [[unlikely]]
		return std::unexpected{ result };
	else
		return result == VK_SUCCESS;
}

std::expected<void, Error> Fence::reset() noexcept
{
	if (auto result = ptr_device->vkResetFences(1, &m_handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	m_signaled = false;
	return {};
}

std::expected<bool, Error> Fence::get_status() const noexcept
{
	if (auto result = ptr_device->vkGetFenceStatus(m_handle); result != VK_SUCCESS ||
		result != VK_NOT_READY) [[unlikely]]
		return std::unexpected{ result };
	else
		return result == VK_SUCCESS;
}

std::expected<Fence, Error> Fence::create(const LogicalDeviceWrapper& device) noexcept
{
	VkFenceCreateInfo createInfo{
		.sType { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO},
		.pNext {nullptr},
		.flags {0}
	};
	VkFence handle{ VK_NULL_HANDLE };
	if (auto result = device.vkCreateFence(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return Fence{device, handle};
}

nyan::vulkan::Fence::Fence(const LogicalDeviceWrapper& device, VkFence handle) noexcept :
	Object(device, handle),
	m_signaled(false)
{
}
