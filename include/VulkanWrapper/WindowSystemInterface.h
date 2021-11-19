#ifndef VKWINDOWSYSTEMINTERFACE_H
#define VKWINDOWSYSTEMINTERFACE_H
#include "VulkanIncludes.h"
namespace vulkan {
	class LogicalDevice;
	class Instance;
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
		VkFormat m_preferredSwapchainFormat = VK_FORMAT_B8G8R8A8_SRGB;
		uint32_t m_swapchainImageIndex = 0;
		bool m_swapchainImageAcquired = false;
		bool m_vsyncEnabled = false;
		bool m_staleSwapchain = true;
		VkFormat m_format = VK_FORMAT_UNDEFINED;
	};
}

#endif //VKWINDOWSYSTEMINTERFACE_H!