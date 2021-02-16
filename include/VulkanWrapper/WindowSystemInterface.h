#ifndef VKWINDOWSYSTEMINTERFACE_H
#define VKWINDOWSYSTEMINTERFACE_H
#include "VulkanIncludes.h"
namespace vulkan {
	class LogicalDevice;
	class Instance;
	class WindowSystemInterface {
	public:
		WindowSystemInterface(LogicalDevice& device, Instance& instance);
		~WindowSystemInterface();
		void drain_swapchain();
		void destroy_swapchain();
		void update_swapchain();
		void begin_frame();
		void end_frame();
		bool init_swapchain();
	private:
		LogicalDevice& r_device;
		Instance& r_instance;
		VkSwapchainKHR m_vkHandle = VK_NULL_HANDLE;
		std::vector<VkImage> m_swapchainImages;
		VkExtent2D m_swapchainExtent{};
		uint32_t m_swapchainImageIndex = 0;
		bool m_swapchainImageAcquired = false;
		VkFormat m_format = VK_FORMAT_UNDEFINED;
	};
}

#endif //VKWINDOWSYSTEMINTERFACE_H!