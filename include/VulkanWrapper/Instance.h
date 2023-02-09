#ifndef VKINSTANCE_H
#define VKINSTANCE_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"

namespace vulkan {
	class Instance {
	public:
		struct Validation {
			uint32_t enabled : 1;
			uint32_t createCallback : 1;
			uint32_t gpuAssisted : 1;
			uint32_t gpuAssistedReserveBindingSlot : 1;
			uint32_t bestPractices : 1;
			uint32_t debugPrintf : 1;
			uint32_t synchronizationValidation : 1;
			uint32_t disableAll : 1;
			uint32_t disableShaders : 1;
			uint32_t disableThreadSafety : 1;
			uint32_t disableAPIParameters : 1;
			uint32_t disableObjectLifetimes : 1;
			uint32_t disableCoreChecks : 1;
			uint32_t disableUniqueHandles : 1;
			uint32_t disableShaderValidationCache : 1;

		};

		Instance(const Validation& validation, const char** extensions, uint32_t extensionCount, std::string applicationName = "", std::string engineName = "");
		~Instance() noexcept;
		Instance(Instance&) = delete;
		Instance& operator=(Instance&) = delete;
		//LogicalDevice setup_device_direct();
		std::unique_ptr<LogicalDevice> setup_device(const std::vector<const char*>& requiredExtensions, const std::vector<const char*>& optionalExtensions);
#ifdef WIN32
		void setup_win32_surface(HWND hwnd, HINSTANCE hinstance);
#else
		void setup_x11_surface(Window window, Display* dpy);
#endif
		std::vector<VkPresentModeKHR> get_present_modes() const;
		std::vector<VkSurfaceFormatKHR> get_surface_formats() const;
		VkSurfaceCapabilitiesKHR get_surface_capabilites() const;
		VkSurfaceKHR get_surface() const;
		operator VkInstance() const noexcept;
		const std::vector<PhysicalDevice>& get_physical_devices() const noexcept;
	private:
		void create_instance(const char** extensions, uint32_t extensionCount, uint32_t applicationVersion = 0, uint32_t engineVersion = 0);

		bool device_swapchain_suitable(const VkPhysicalDevice& device) const;
		bool is_device_suitable(const VkPhysicalDevice& device) const;

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************

		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		size_t m_bestDeviceIdx = 0;
		std::vector<PhysicalDevice> m_physicalDevices;
		VkAllocationCallbacks* m_allocator = NULL;
		VkDebugReportCallbackEXT m_debugReport {VK_NULL_HANDLE};

		Validation m_validation;
		std::vector<const char*> m_layers;
		std::vector<const char*> m_extensions;
		std::string m_applicationName;
		std::string m_engineName;
		friend class LogicalDevice;
	};
	class Surface {
	public:

	private:
		std::vector<VkSurfaceFormatKHR> m_surfaceFormats;
		std::vector<VkPresentModeKHR> m_presentModes;
	};
	struct Extensions {
		uint32_t swapchain : 1;
		uint32_t fullscreen_exclusive : 1;
		uint32_t debug_utils : 1;
		uint32_t debug_marker : 1;
		uint32_t acceleration_structure : 1;
		uint32_t ray_tracing_pipeline : 1;
		uint32_t ray_query : 1;
		uint32_t pipeline_library : 1;
		uint32_t deferred_host_operations : 1;
		uint32_t performance_query : 1;
		uint32_t vertex_input_dynamic_state : 1;
		uint32_t mesh_shader : 1;
		uint32_t atomic_floats : 1;
		uint32_t buffer_device_address : 1;
	};
}
#endif