#pragma once
#include <span>
#include <expected>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"

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
		Queue(LogicalDevice& device, Type type, VkQueue queueHandle, uint32_t queueFamilyIndex, float priority) noexcept;
		std::expected<void, vulkan::Error> wait_idle() const noexcept;
		std::expected<void, vulkan::Error> submit2(std::span<VkSubmitInfo2> submits, VkFence fence = VK_NULL_HANDLE) const noexcept;
		std::expected<void, vulkan::Error> present(Swapchain& swapchain, std::span<VkSemaphore> waitSemaphores) const noexcept;
		void set_present_capable(bool capable) noexcept;
		[[nodiscard]] float get_priority() const noexcept
		{
			return m_priority;
		}

		[[nodiscard]] uint32_t get_family_index() const noexcept
		{
			return m_queueFamilyIndex;
		}
		[[nodiscard]] bool get_present_capable() const noexcept
		{
			return m_presentCapable;
		}
		[[nodiscard]] Type get_type() const noexcept
		{
			return m_type;
		}
	private:
		Type m_type;
		uint32_t m_queueFamilyIndex;
		float m_priority;
		bool m_presentCapable;
	};
}