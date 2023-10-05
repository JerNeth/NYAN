#pragma once

#include <expected>

#include "VulkanIncludes.h"

#include "VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
namespace vulkan
{
	struct DescriptorCreateInfo {
		uint32_t storage_buffer_count{ 0 };
		uint32_t uniform_buffer_count{ 0 };
		uint32_t sampler_count{ 0 };
		uint32_t sampled_image_count{ 0 };
		uint32_t storage_image_count{ 0 };
		uint32_t acceleration_structure_count{ 0 };
	};
	class DescriptorPool : public VulkanObject<VkDescriptorPool> {
	public:
		friend class DescriptorSet;
		DescriptorPool(LogicalDevice& device);
		DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo);
		~DescriptorPool() noexcept;
		DescriptorPool(DescriptorPool&) = delete;

		DescriptorPool(DescriptorPool&& other) noexcept;

		DescriptorPool& operator=(DescriptorPool&) = delete;

		DescriptorPool& operator=(DescriptorPool&& other) noexcept;

		//void get_set()
		DescriptorSet allocate_set();
		const DescriptorCreateInfo& get_info() const;
		VkDescriptorSetLayout get_layout() const;

		static std::expected<DescriptorPool, VkResult> create_descriptor_pool(LogicalDevice& device);
	private:
		VkDescriptorSetLayout m_layout;
		DescriptorCreateInfo m_createInfo;
	};
}