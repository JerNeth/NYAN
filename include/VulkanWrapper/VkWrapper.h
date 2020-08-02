#ifndef VKWRAPPER_H
#define VKWRAPPER_H
#pragma once
#ifdef NDEBUG
constexpr bool debug = false;
#else
constexpr bool debug = true;
#endif
#define GLFW_INCLUDE_VULKAN
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <optional>
#include <bitset>
#include <vector>
#include <stdexcept>
#include <iostream>
namespace vk {
	class Instance {
	public:
		Instance(const char** extensions, uint32_t extensionCount, std::string applicationName = "", std::string engineName = ""): m_applicationName(applicationName), m_engineName(engineName) {
			m_extensions.assign(extensions, extensions + extensionCount);
			create_instance();
		}
		~Instance() {
			for (auto imageView : m_swapChainImageViews)
				vkDestroyImageView(m_device, imageView, m_allocator);
			vkDestroySwapchainKHR(m_device, m_swapChain, m_allocator);
			vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
			vkDestroyDevice(m_device, nullptr);
			vkDestroyInstance(m_instance, m_allocator);
		}
		void setup_device();
		void setup_win32_surface(HWND hwnd, HINSTANCE hinstance);
		void create_swapchain();
		void create_graphics_pipeline();
	private:
		void create_instance();
		void create_image_views();
		bool device_supports_features(const VkPhysicalDevice& device) const;
		bool device_supports_extensions(const VkPhysicalDevice& device) const;
		bool device_has_properties(const VkPhysicalDevice& device) const;
		bool device_swapchain_suitable(const VkPhysicalDevice& device) const;
		bool is_device_suitable(const VkPhysicalDevice& device) const;
		uint32_t get_graphics_family_queue_index(const VkPhysicalDevice& device) const;
		std::pair<VkDevice, uint32_t> setup_logical_device(const VkPhysicalDevice& device) const;
		
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		std::optional<std::bitset<64>> m_requiredFeatures = std::nullopt;
		const std::vector<const char*> m_requiredExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkAllocationCallbacks* m_allocator = NULL;
		VkDevice m_device;
		VkQueue m_graphicsQueue;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkExtent2D m_swapChainExtent;
		VkFormat m_swapChainImageFormat;
		std::vector<VkImageView> m_swapChainImageViews;

		std::vector<const char*> m_layers;
		std::vector<const char*> m_extensions;
		std::string m_applicationName;
		std::string m_engineName;

	};
	
}

#endif // VKWRAPPER_H!
