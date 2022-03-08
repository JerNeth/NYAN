#ifndef VKINSTANCE_H
#define VKINSTANCE_H
#pragma once
#include "VulkanIncludes.h"
#include "LogicalDevice.h"

namespace vulkan {
	class PhysicalDevice;
	class LogicalDevice;
	class Instance {
	public:
		Instance(const char** extensions, uint32_t extensionCount, std::string applicationName = "", std::string engineName = "");
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
	private:
		void create_instance(uint32_t applicationVersion = 0, uint32_t engineVersion = 0);

		bool device_swapchain_suitable(const VkPhysicalDevice& device) const;
		bool is_device_suitable(const VkPhysicalDevice& device) const;

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************

		VkInstance m_instance;
		VkSurfaceKHR m_surface;
		VkPhysicalDevice m_physicalDevice;
		std::vector<PhysicalDevice> m_physicalDevices;
		VkAllocationCallbacks* m_allocator = NULL;
		VkDebugReportCallbackEXT m_debugReport;


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
		uint32_t vertex_input_dynamic_state : 1;
		uint32_t mesh_shader : 1;
		uint32_t buffer_device_address : 1;
	};
	class PhysicalDevice {
	public:
		PhysicalDevice(VkPhysicalDevice device);
		operator VkPhysicalDevice() const noexcept;
		const VkPhysicalDevice& get_handle() const noexcept;

		bool use_extension(const char* extension) noexcept;
		std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance, uint32_t genericQueueCount = 1, uint32_t transferQueueCount = 1, uint32_t computeQueueCount = 1);

		uint32_t get_generic_queue_family() const noexcept;
		uint32_t get_transfer_queue_family() const noexcept;
		uint32_t get_compute_queue_family() const noexcept;

		const std::vector<const char*>& get_used_extensions() const noexcept;
		const VkPhysicalDeviceFeatures& get_used_features() const noexcept;
		const VkPhysicalDeviceAccelerationStructureFeaturesKHR& get_acceleration_structure_features() const noexcept;
		const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& get_ray_tracing_pipeline_features() const noexcept;
		const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& get_vertex_input_dynamic_state_features() const noexcept;
		const VkPhysicalDeviceMeshShaderFeaturesNV& get_mesh_shader_features() const noexcept;

		const VkPhysicalDeviceProperties& get_properties() const noexcept;
		const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const noexcept;
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const noexcept;
		const VkPhysicalDeviceMeshShaderPropertiesNV& get_mesh_shader_properties() const noexcept;

		const Extensions& get_extensions() const noexcept;
	private:
		void init_queues() noexcept;
		void init_extensions() noexcept;
		void init_features() noexcept;
		void init_properties() noexcept;


		VkPhysicalDevice m_vkHandle = VK_NULL_HANDLE;
		uint32_t m_genericQueueFamily = ~0u;
		uint32_t m_transferQueueFamily = ~0u;
		uint32_t m_computeQueueFamily = ~0u;
		Extensions m_extensions;
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		std::vector<VkExtensionProperties> m_availableExtensions;
		std::vector<const char*> m_usedExtensions;
		VkPhysicalDeviceFeatures2 m_features;
		VkPhysicalDeviceVulkan11Features  m_11Features;
		VkPhysicalDeviceVulkan12Features  m_12Features;
		VkPhysicalDeviceVulkan13Features  m_13Features;
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures;
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures;
		VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT m_vertexInputDynamicStateFeatures;
		VkPhysicalDeviceMeshShaderFeaturesNV m_meshShaderFeatures;

		VkPhysicalDeviceProperties2 m_properties;
		VkPhysicalDeviceVulkan11Properties  m_11Properties;
		VkPhysicalDeviceVulkan12Properties  m_12Properties;
		VkPhysicalDeviceVulkan13Properties  m_13Properties;
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties;
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties;
		VkPhysicalDeviceMeshShaderPropertiesNV m_meshShaderProperties;

	};
}
#endif