#include "CommandPool.h"
#include "LogicalDevice.h"
#include "Utility/Exceptions.h"

vulkan::CommandPool::CommandPool(LogicalDevice& parent, uint32_t queueFamilyIndex) :
	r_device(parent)
{
	VkCommandPoolCreateInfo commandPoolCreateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
			.queueFamilyIndex = queueFamilyIndex,
	};
	if (auto result = vkCreateCommandPool(r_device.get_device(), &commandPoolCreateInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
}

vulkan::CommandPool::CommandPool(CommandPool&& other) noexcept :
	r_device(other.r_device),
	m_vkHandle(other.m_vkHandle),
	m_primaryBuffers(other.m_primaryBuffers),
	m_secondaryBuffers(other.m_secondaryBuffers),
	m_primaryBufferIdx(other.m_primaryBufferIdx),
	m_secondaryBufferIdx(other.m_secondaryBufferIdx)
{
	other.m_vkHandle = VK_NULL_HANDLE;
}

vulkan::CommandPool::~CommandPool() noexcept
{
	if (m_vkHandle != VK_NULL_HANDLE) {
		vkDestroyCommandPool(r_device.get_device(), m_vkHandle, r_device.get_allocator());
	}
}

VkCommandBuffer vulkan::CommandPool::request_command_buffer()
{
	if (m_primaryBufferIdx < m_primaryBuffers.size()) {
		//std::cout << "Requested (Reuse) Cmd Buffer: " << m_primaryBuffers[m_primaryBufferIdx] << "\n";
		return m_primaryBuffers[m_primaryBufferIdx++];
	}
	else {
		VkCommandBuffer buffer {VK_NULL_HANDLE};
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_vkHandle,
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = 1,
		}; 
		if (auto result = vkAllocateCommandBuffers(r_device.get_device(), &commandBufferAllocateInfo, &buffer); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
		m_primaryBuffers.push_back(buffer);
		//std::cout << "Requested (Create) Cmd Buffer: " << m_primaryBuffers[m_primaryBufferIdx] << "\n";
		m_primaryBufferIdx++;
		return buffer;
	}
}

VkCommandBuffer vulkan::CommandPool::request_secondary_command_buffer()
{
	if (m_secondaryBufferIdx < m_secondaryBuffers.size()) {
		return m_secondaryBuffers[m_secondaryBufferIdx++];
	}
	else {
		VkCommandBuffer buffer{ VK_NULL_HANDLE };
		VkCommandBufferAllocateInfo commandBufferAllocateInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_vkHandle,
			.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY,
			.commandBufferCount = 1,
		};
		if (auto result = vkAllocateCommandBuffers(r_device.get_device(), &commandBufferAllocateInfo, &buffer); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
		m_secondaryBuffers.push_back(buffer);
		m_secondaryBufferIdx++;
		return buffer;
	}
}


void vulkan::CommandPool::reset()
{
	if (m_primaryBufferIdx > 0 || m_secondaryBufferIdx > 0)
		vkResetCommandPool(r_device.get_device(), m_vkHandle, 0);
	m_primaryBufferIdx = 0;
	m_secondaryBufferIdx = 0;
}
