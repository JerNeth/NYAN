#pragma once
#include "VulkanForwards.h"
#include "VulkanIncludes.h"

namespace vulkan
{
	class Swapchain : VulkanObject<VkSwapchainKHR>
	{
	public:
		Swapchain(vulkan::LogicalDevice& device, VkSurfaceKHR surface);
		Swapchain(const Swapchain& other) = delete;
		Swapchain(Swapchain&& other) noexcept;
		~Swapchain();
		const Swapchain& operator=(Swapchain&) = delete;
		const Swapchain& operator=(Swapchain&&) = delete;

		void acquire_image();
		void acquire_image_async();
		

	private:
		void create_swapchain();
		std::vector<uint32_t> m_acquiredImages{};
		std::vector<VkImage> m_swapchainImages{};
		VkSurfaceCapabilitiesKHR m_surfaceCapabilites{};
		VkSurfaceKHR m_surface{VK_NULL_HANDLE};
	};
}