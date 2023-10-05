#pragma once
#ifndef VKWINDOWSYSTEMINTERFACE_H
#define VKWINDOWSYSTEMINTERFACE_H

#include <vector>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"

namespace vulkan {
	class WindowSystemInterface {
	public:
		WindowSystemInterface(LogicalDevice& device, Instance& instance, VkPresentModeKHR preferedPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR);
		~WindowSystemInterface();
		void drain_swapchain();
		void destroy_swapchain();
		void update_swapchain();
		void begin_frame();
		void end_frame();
		void set_vsync(bool enabled);
		bool init_swapchain();
		void set_preferred_present_mode(VkPresentModeKHR presentMode) noexcept {
			m_staleSwapchain |= (m_preferredPresentMode != presentMode);
			m_preferredPresentMode = presentMode;
		}
		VkPresentModeKHR get_preferred_present_mode() const noexcept {
			return m_preferredPresentMode;
		}
		void set_preferred_format(VkFormat format) noexcept {
			m_staleSwapchain |= (m_preferredSwapchainFormat != format);
			m_preferredSwapchainFormat = format;
		}
		VkFormat get_preferred_format() const noexcept {
			return m_preferredSwapchainFormat;
		}
	private:
		LogicalDevice& r_device;
		Instance& r_instance;
		VkSwapchainKHR m_vkHandle = VK_NULL_HANDLE;
		std::vector<VkImage> m_swapchainImages;
		VkExtent2D m_swapchainExtent{};
		VkPresentModeKHR m_preferredPresentMode = VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR;
		VkFormat m_preferredSwapchainFormat = VK_FORMAT_A2B10G10R10_UNORM_PACK32;
		//VkFormat m_preferredSwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;//VK_FORMAT_A2B10G10R10_UNORM_PACK32;//VK_FORMAT_B8G8R8A8_UNORM
		uint32_t m_swapchainImageIndex = 0;
		bool m_swapchainImageAcquired = false;
		bool m_vsyncEnabled = false;
		bool m_staleSwapchain = true;
		VkFormat m_format = VK_FORMAT_UNDEFINED;
		VkImageUsageFlags m_usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	};
}

#endif //VKWINDOWSYSTEMINTERFACE_H!