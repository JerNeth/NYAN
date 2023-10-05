#ifndef VKCOMMANDPOOL_H
#define VKCOMMANDPOOL_H
#pragma once

#include <vector>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"

namespace vulkan {
	class CommandPool {
	public:
		CommandPool(LogicalDevice& parent, uint32_t queueFamilyIndex);
		CommandPool(CommandPool&) = delete;
		CommandPool(CommandPool&&) noexcept;
		CommandPool& operator=(CommandPool&) = delete;
		CommandPool& operator=(CommandPool&&) = delete;
		~CommandPool() noexcept;
		VkCommandBuffer request_command_buffer();
		VkCommandBuffer request_secondary_command_buffer();
		void reset();
	private:
		LogicalDevice& r_device;
		VkCommandPool m_vkHandle =VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_primaryBuffers;
		std::vector<VkCommandBuffer> m_secondaryBuffers;
		uint32_t m_primaryBufferIdx = 0;
		uint32_t m_secondaryBufferIdx = 0;
	};
}
#endif