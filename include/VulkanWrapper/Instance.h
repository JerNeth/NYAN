#ifndef VKINSTANCE_H
#define VKINSTANCE_H
#pragma once
#include "VulkanIncludes.h"
#include "LogicalDevice.h"

namespace vulkan {
	class LogicalDevice;
	class Instance {
	public:
		Instance(const char** extensions, uint32_t extensionCount, std::string applicationName = "", std::string engineName = "") : m_applicationName(applicationName), m_engineName(engineName) {
			volkInitialize();
			m_extensions.assign(extensions, extensions + extensionCount);
			create_instance();

		}
		~Instance() {
			if constexpr (debug) {
				//auto vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT");
				vkDestroyDebugReportCallbackEXT(m_instance, m_debugReport, m_allocator);
			}
			vkDestroySurfaceKHR(m_instance, m_surface, m_allocator);
			vkDestroyInstance(m_instance, m_allocator);
			
		}
		Instance(Instance&) = delete;
		Instance& operator=(Instance&) = delete;
		//LogicalDevice setup_device_direct();
		std::unique_ptr<LogicalDevice> setup_device();
#ifdef WIN32
		void setup_win32_surface(HWND hwnd, HINSTANCE hinstance);
#else
		void setup_x11_surface(Window window, Display* dpy);
#endif
		uint32_t find_memory_type(uint32_t typeFilter, VkMemoryPropertyFlags properties) const;
		std::vector<VkPresentModeKHR> get_present_modes() const;
		std::vector<VkSurfaceFormatKHR> get_surface_formats() const;
		VkSurfaceCapabilitiesKHR get_surface_capabilites() const;
		VkSurfaceKHR get_surface() const;
	private:
		void create_instance();
		
		bool device_supports_features(const VkPhysicalDevice& device) const;
		bool device_supports_extensions(const VkPhysicalDevice& device) const;
		bool device_has_properties(const VkPhysicalDevice& device) const;
		bool device_swapchain_suitable(const VkPhysicalDevice& device) const;
		bool is_device_suitable(const VkPhysicalDevice& device) const;
		uint32_t get_graphics_family_queue_index(const VkPhysicalDevice& device) const;
		uint32_t get_compute_family_queue_index(const VkPhysicalDevice& device) const;
		uint32_t get_transfer_family_queue_index(const VkPhysicalDevice& device) const;
		std::tuple<VkDevice, uint32_t, uint32_t, uint32_t> setup_logical_device(const VkPhysicalDevice& device) const;

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		std::optional<std::bitset<64>> m_requiredFeatures = std::nullopt;
		std::vector<const char*> m_requiredExtensions = {
				VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};
		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		VkAllocationCallbacks* m_allocator = NULL;
		VkDebugReportCallbackEXT m_debugReport;


		std::vector<const char*> m_layers;
		std::vector<const char*> m_extensions;
		std::string m_applicationName;
		std::string m_engineName;
		friend class LogicalDevice;
	};
}
#endif