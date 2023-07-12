#pragma once
#include "VulkanForwards.h"
#include "VulkanIncludes.h"
#include <span>

namespace vulkan
{
	class Queue : VulkanObject<VkQueue>
	{
	public:
		enum class Type : uint32_t
		{
			Graphics,
			Compute,
			Transfer,
			Encode,
			Decode,
			Size
		};
	public:
		Queue(LogicalDevice& device, Type type, VkQueue queueHandle, uint32_t queueFamilyIndex, float priority);
		void wait_idle() const;
		void submit2(std::span<VkSubmitInfo2> submits, VkFence fence = VK_NULL_HANDLE) const;
		void present(Swapchain& swapchain, std::span<VkSemaphore> waitSemaphores) const;
		void set_present_capable(bool capable) noexcept;
		float get_priority() const noexcept;
		uint32_t get_family_index() const noexcept;
		bool get_present_capable() const noexcept;
		vulkan::Queue::Type get_type() const noexcept;
	private:
		Type m_type;
		uint32_t m_queueFamilyIndex;
		float m_priority;
		bool m_presentCapable;
	};
}