#pragma once
#include "VulkanForwards.h"
#include "VulkanIncludes.h"
#include <span>

namespace vulkan
{
	class Queue : VulkanObject<VkQueue>
	{
	public:
		enum class Type
		{
			Generic,
			Compute,
			Transfer
		};
	public:
		explicit Queue(LogicalDevice& device);
		void wait_idle() const;
		void submit2(std::span<VkSubmitInfo2> submits, VkFence fence = VK_NULL_HANDLE) const;
	private:

	};
}