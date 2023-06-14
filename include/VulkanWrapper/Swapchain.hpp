#pragma once
#include "VulkanForwards.h"
#include "VulkanIncludes.h"

namespace vulkan
{
	class Swapchain : VulkanObject<VkSwapchainKHR>
	{
	public:
		explicit Swapchain(vulkan::LogicalDevice& device);
		Swapchain(const Swapchain& other) = delete;
		Swapchain(Swapchain&& other) noexcept;
		~Swapchain();
		const Swapchain& operator=(Swapchain&) = delete;
		const Swapchain& operator=(Swapchain&&) = delete;

		void acquire_image();
		

	private:
		void create_swapchain();
		std::vector<uint32_t> m_acquiredImages{};
		std::vector<VkImage> m_swapchainImages{};
		VkSurfaceCapabilitiesKHR m_surfaceCapabilites{};

	};
}