module;

//#include <array>
//#include <cassert>
//#include <expected>
//#include <utility>

#include "volk.h"

module NYANVulkan;
import std;

using namespace nyan::vulkan;


CommandPool::CommandPool(CommandPool&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	r_queue(other.r_queue)
{

}

CommandPool& CommandPool::operator=(CommandPool&& other) noexcept
{
	::assert(ptr_device == other.ptr_device);
	::assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	::assert(std::addressof(r_queue) == std::addressof(other.r_queue));
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

CommandPool::~CommandPool() noexcept
{
	if (m_handle != VK_NULL_HANDLE) {
		r_deletionQueue.queue_deletion(m_handle);
	}
}

std::expected<void, Error> CommandPool::reset(bool release) noexcept
{
	if(auto result = ptr_device->vkResetCommandPool(m_handle, release? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT: static_cast<VkCommandPoolResetFlagBits>(0)); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<CommandBuffer, Error> CommandPool::allocate_command_buffer() noexcept
{
	VkCommandBufferAllocateInfo allocateInfo {
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO},
		.pNext{ nullptr },
		.commandPool {m_handle},
		.level { VK_COMMAND_BUFFER_LEVEL_PRIMARY },
		.commandBufferCount {1}
	};
	VkCommandBuffer cmd{ VK_NULL_HANDLE };
	if (auto result = ptr_device->vkAllocateCommandBuffers(&allocateInfo, &cmd); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return CommandBufferAccessor::create(*ptr_device, cmd, r_queue);
}

std::expected<CommandPool, Error> CommandPool::create(LogicalDevice& device, Queue& queue, bool transient, bool reset) noexcept
{
	VkCommandPool handle{ VK_NULL_HANDLE };
	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = (transient? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : static_cast<VkCommandPoolCreateFlags>(0)) |
				(reset? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : static_cast<VkCommandPoolCreateFlags>(0)),
		.queueFamilyIndex = queue.get_queue_family_index().value,
	};
	if (auto result = device.get_device().vkCreateCommandPool(&commandPoolCreateInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return CommandPool{device.get_device(), handle, device.get_deletion_queue(), queue};
}

CommandPool::CommandPool(const LogicalDeviceWrapper& device, VkCommandPool handle, DeletionQueue& deletionQueue, Queue& queue) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	r_queue(queue)
{
	::assert(m_handle != VK_NULL_HANDLE);
}
