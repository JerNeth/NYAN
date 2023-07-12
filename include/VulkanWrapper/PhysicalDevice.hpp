#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"

namespace vulkan {
	class PhysicalDevice {
	public:
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
	public:
		explicit PhysicalDevice(VkPhysicalDevice device);
		[[nodiscard]] operator VkPhysicalDevice() const noexcept;
		[[nodiscard]] const VkPhysicalDevice& get_handle() const noexcept;

		[[nodiscard]] bool use_extension(const char* extension) noexcept;
		[[nodiscard]] std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance);
		[[nodiscard]] std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance, const std::vector<float>& genericQueuePriorities, const std::vector<float>& computeQueuePriorities, const std::vector<float>& transferQueuePriorities);
		
		[[nodiscard]] bool supports_surface(VkSurfaceKHR surface, uint32_t queueFamilyIndex) const;
		[[nodiscard]] std::vector<VkPresentModeKHR> get_present_modes(VkSurfaceKHR surface) const;
		[[nodiscard]] std::vector<VkSurfaceFormat2KHR> get_surface_formats2(VkSurfaceKHR surface) const;
		[[nodiscard]] VkSurfaceCapabilities2KHR get_surface_capabilites2(VkSurfaceKHR surface) const;

		[[nodiscard]] const std::vector<const char*>& get_used_extensions() const noexcept;
		[[nodiscard]] const std::vector<VkQueueFamilyProperties>& get_queue_family_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceFeatures& get_used_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan11Features& get_vulkan11_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan12Features& get_vulkan12_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan13Features& get_vulkan13_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& get_atomic_float_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceAccelerationStructureFeaturesKHR& get_acceleration_structure_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& get_ray_tracing_pipeline_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayQueryFeaturesKHR& get_ray_query_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& get_vertex_input_dynamic_state_features() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMeshShaderFeaturesEXT& get_mesh_shader_features() const noexcept;

		[[nodiscard]] const VkPhysicalDeviceProperties& get_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceSubgroupProperties& get_subgroup_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan11Properties& get_vulkan11_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan12Properties& get_vulkan12_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceVulkan13Properties& get_vulkan13_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const noexcept;
		[[nodiscard]] const VkPhysicalDeviceMeshShaderPropertiesEXT& get_mesh_shader_properties() const noexcept;

		[[nodiscard]] const Extensions& get_extensions() const noexcept;
		[[nodiscard]] std::optional<VkImageFormatProperties> get_image_format_properties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const noexcept;
	private:
		void init_queues() noexcept;
		void init_extensions() noexcept;
		void init_features() noexcept;
		void init_properties() noexcept;
		void update_feature_chain() noexcept;


		VkPhysicalDevice m_vkHandle = VK_NULL_HANDLE;
		uint32_t m_genericQueueFamily = ~0u;
		uint32_t m_transferQueueFamily = ~0u;
		uint32_t m_computeQueueFamily = ~0u;
		Extensions m_extensions{};
		std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
		std::vector<VkExtensionProperties> m_availableExtensions;
		std::vector<const char*> m_usedExtensions;
		VkPhysicalDeviceFeatures2 m_features{};
		VkPhysicalDeviceVulkan11Features  m_11Features{};
		VkPhysicalDeviceVulkan12Features  m_12Features{};
		VkPhysicalDeviceVulkan13Features  m_13Features{};
		VkPhysicalDeviceShaderAtomicFloatFeaturesEXT m_atomicFloatFeatures{};
		VkPhysicalDeviceAccelerationStructureFeaturesKHR m_accelerationStructureFeatures{};
		VkPhysicalDeviceRayTracingPipelineFeaturesKHR m_rayTracingPipelineFeatures{};
		VkPhysicalDeviceRayQueryFeaturesKHR m_rayQueryFeatures{};
		VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT m_vertexInputDynamicStateFeatures{};
		VkPhysicalDeviceMeshShaderFeaturesEXT m_meshShaderFeatures{};
		VkPhysicalDevicePresentIdFeaturesKHR  m_presentIdFeatures{};
		VkPhysicalDevicePresentWaitFeaturesKHR   m_presentWaitFeatures{};

		VkPhysicalDeviceProperties2 m_properties{};
		VkPhysicalDeviceSubgroupProperties m_subgroupProperties{};
		VkPhysicalDeviceVulkan11Properties  m_11Properties{};
		VkPhysicalDeviceVulkan12Properties  m_12Properties{};
		VkPhysicalDeviceVulkan13Properties  m_13Properties{};
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties{};
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties{};
		VkPhysicalDeviceMeshShaderPropertiesEXT m_meshShaderProperties{};

	};
}