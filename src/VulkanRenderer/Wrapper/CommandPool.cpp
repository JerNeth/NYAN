module;

#include <array>
#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;

using namespace nyan::vulkan::wrapper;


nyan::vulkan::wrapper::CommandPool::CommandPool(CommandPool&& other) noexcept :
	Object(other.r_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue),
	r_queue(other.r_queue)
{

}

CommandPool& nyan::vulkan::wrapper::CommandPool::operator=(CommandPool&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	assert(std::addressof(r_queue) == std::addressof(other.r_queue));
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

nyan::vulkan::wrapper::CommandPool::~CommandPool() noexcept
{
	if (m_handle != VK_NULL_HANDLE) {
		r_deletionQueue.queue_command_pool_deletion(m_handle);
	}
}

std::expected<void, Error> nyan::vulkan::wrapper::CommandPool::reset(bool release) noexcept
{
	if(auto result = r_device.vkResetCommandPool(m_handle, release? VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT: static_cast<VkCommandPoolResetFlagBits>(0)); result != VK_SUCCESS)
		return std::unexpected{ result };

	return {};
}

std::expected<CommandBuffer, Error> nyan::vulkan::wrapper::CommandPool::allocate_command_buffer() noexcept
{
	VkCommandBufferAllocateInfo allocateInfo {
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO},
		.pNext{ nullptr },
		.commandPool {m_handle},
		.level {VK_COMMAND_BUFFER_LEVEL_PRIMARY },
		.commandBufferCount {1}
	};
	VkCommandBuffer cmd{ VK_NULL_HANDLE };
	if (auto result = r_device.vkAllocateCommandBuffers(&allocateInfo, &cmd); result != VK_SUCCESS)
		return std::unexpected{ result };

	return CommandBufferAccessor::create(r_device, cmd, r_queue);
}

std::expected<CommandPool, Error> CommandPool::create(LogicalDevice& device, Queue& queue, bool transient, bool reset) noexcept
{
	VkCommandPool handle{ VK_NULL_HANDLE };
	VkCommandPoolCreateInfo commandPoolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = (transient? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : static_cast<VkCommandPoolCreateFlags>(0)) |
				(reset? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : static_cast<VkCommandPoolCreateFlags>(0)),
		.queueFamilyIndex = queue.get_queue_family_index(),
	};
	if (auto result = device.get_device().vkCreateCommandPool(&commandPoolCreateInfo, &handle); result != VK_SUCCESS)
		return std::unexpected{ result };

	return CommandPool{device.get_device(), handle, device.get_deletion_queue(), queue};
}

CommandPool::CommandPool(const LogicalDeviceWrapper& device, VkCommandPool handle, DeletionQueue& deletionQueue, Queue& queue) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	r_queue(queue)
{
	assert(m_handle != VK_NULL_HANDLE);
}
