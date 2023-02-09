#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"

namespace vulkan {
	class PhysicalDevice {
	public:
		explicit PhysicalDevice(VkPhysicalDevice device);
		operator VkPhysicalDevice() const noexcept;
		const VkPhysicalDevice& get_handle() const noexcept;

		bool use_extension(const char* extension) noexcept;
		std::unique_ptr<LogicalDevice> create_logical_device(const Instance& instance, uint32_t genericQueueCount = 1, uint32_t transferQueueCount = 1, uint32_t computeQueueCount = 1);

		bool supports_ray_pipelines() const noexcept;

		uint32_t get_generic_queue_family() const noexcept;
		uint32_t get_transfer_queue_family() const noexcept;
		uint32_t get_compute_queue_family() const noexcept;

		const std::vector<const char*>& get_used_extensions() const noexcept;
		const std::vector<VkQueueFamilyProperties>& get_queue_family_properties() const noexcept;
		const VkPhysicalDeviceFeatures& get_used_features() const noexcept;
		const VkPhysicalDeviceVulkan11Features& get_vulkan11_features() const noexcept;
		const VkPhysicalDeviceVulkan12Features& get_vulkan12_features() const noexcept;
		const VkPhysicalDeviceVulkan13Features& get_vulkan13_features() const noexcept;
		const VkPhysicalDeviceShaderAtomicFloatFeaturesEXT& get_atomic_float_features() const noexcept;
		const VkPhysicalDeviceAccelerationStructureFeaturesKHR& get_acceleration_structure_features() const noexcept;
		const VkPhysicalDeviceRayTracingPipelineFeaturesKHR& get_ray_tracing_pipeline_features() const noexcept;
		const VkPhysicalDeviceRayQueryFeaturesKHR& get_ray_query_features() const noexcept;
		const VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT& get_vertex_input_dynamic_state_features() const noexcept;
		const VkPhysicalDeviceMeshShaderFeaturesNV& get_mesh_shader_features() const noexcept;

		const VkPhysicalDeviceProperties& get_properties() const noexcept;
		const VkPhysicalDeviceSubgroupProperties& get_subgroup_properties() const noexcept;
		const VkPhysicalDeviceVulkan11Properties& get_vulkan11_properties() const noexcept;
		const VkPhysicalDeviceVulkan12Properties& get_vulkan12_properties() const noexcept;
		const VkPhysicalDeviceVulkan13Properties& get_vulkan13_properties() const noexcept;
		const VkPhysicalDeviceAccelerationStructurePropertiesKHR& get_acceleration_structure_properties() const noexcept;
		const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& get_ray_tracing_pipeline_properties() const noexcept;
		const VkPhysicalDeviceMeshShaderPropertiesNV& get_mesh_shader_properties() const noexcept;

		const Extensions& get_extensions() const noexcept;
		std::optional<VkImageFormatProperties> get_image_format_properties(VkFormat format, VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags) const noexcept;
	private:
		void init_queues() noexcept;
		void init_extensions() noexcept;
		void init_features() noexcept;
		void init_properties() noexcept;


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
		VkPhysicalDeviceMeshShaderFeaturesNV m_meshShaderFeatures{};

		VkPhysicalDeviceProperties2 m_properties{};
		VkPhysicalDeviceSubgroupProperties m_subgroupProperties{};
		VkPhysicalDeviceVulkan11Properties  m_11Properties{};
		VkPhysicalDeviceVulkan12Properties  m_12Properties{};
		VkPhysicalDeviceVulkan13Properties  m_13Properties{};
		VkPhysicalDeviceAccelerationStructurePropertiesKHR m_accelerationStructureProperties{};
		VkPhysicalDeviceRayTracingPipelinePropertiesKHR m_rayTracingPipelineProperties{};
		VkPhysicalDeviceMeshShaderPropertiesNV m_meshShaderProperties{};

	};
}