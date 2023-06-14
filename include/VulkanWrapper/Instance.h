#ifndef VKINSTANCE_H
#define VKINSTANCE_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <span>

namespace vulkan {
	class Instance {
	public:
		struct Validation {
			uint32_t enabled : 1;
			uint32_t createCallback : 1;
			uint32_t callBackVerbose : 1;
			uint32_t callBackInfo : 1;
			uint32_t callBackWarning : 1;
			uint32_t callBackError : 1;
			uint32_t callBackGeneral : 1;
			uint32_t callBackValidation : 1;
			uint32_t callBackPerformance : 1;
			uint32_t callBackDeviceAddressBinding : 1;
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
		struct Extensions {
			uint32_t debugUtils : 1;
			uint32_t debugReport : 1;
			uint32_t validationFeatures : 1;
			uint32_t display : 1;
			uint32_t surface : 1;
			uint32_t surfaceWin32 : 1;
			uint32_t surfaceX11 : 1;
			uint32_t surfaceCapabilites2 : 1;
			uint32_t swapchainColorSpace : 1;
		};

		Instance(const Validation& validation, std::span<const char*> requiredExtensions, std::span<const char*> optionalExtensions, std::string applicationName = "", std::string engineName = "");
		~Instance() noexcept;
		Instance(Instance&) = delete;
		Instance& operator=(Instance&) = delete;
		//LogicalDevice setup_device_direct();
		std::unique_ptr<LogicalDevice> setup_device(const std::span<const char*>& requiredExtensions, const std::span<const char*>& optionalExtensions);
#ifdef WIN32
		void setup_win32_surface(HWND hwnd, HINSTANCE hinstance);
#else
		void setup_x11_surface(Window window, Display* dpy);
#endif
		VkSurfaceKHR get_surface() const;
		operator VkInstance() const noexcept;
		const std::vector<PhysicalDevice>& get_physical_devices() const noexcept;
	private:
		void create_instance( std::span<const char*> requiredExtensions, std::span<const char*> optionalExtensions, uint32_t applicationVersion = 0, uint32_t engineVersion = 0);

		bool use_extension(const char* extension);
		bool use_layer(const char* layer);
		void init_layers();
		void init_extensions();

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************

		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		size_t m_bestDeviceIdx = 0;
		std::vector<PhysicalDevice> m_physicalDevices;
		VkAllocationCallbacks* m_allocator = NULL;
		VkDebugUtilsMessengerEXT m_debugMessenger {VK_NULL_HANDLE};

		Validation m_validation;
		std::vector<const char*> m_usedExtensions;
		std::vector<const char*> m_layers;
		std::vector<VkLayerProperties> m_availableLayers;
		std::vector<VkExtensionProperties> m_availableExtensions;
		Extensions m_extensions;
		
		std::string m_applicationName;
		std::string m_engineName;
		friend class LogicalDevice;
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
		uint32_t present_id : 1;
		uint32_t present_wait : 1;
	};
}
#endif