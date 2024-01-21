#pragma once

#include <expected>

#include "VulkanIncludes.h"

#include "VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"
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
	private:
		DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo, VkDescriptorSetLayout layout, VkDescriptorPool pool) noexcept;
	public:
		~DescriptorPool() noexcept;
		DescriptorPool(DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;
		DescriptorPool& operator=(DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&& other) noexcept;

		//void get_set()
		[[nodiscard]] std::expected<DescriptorSet, vulkan::Error> allocate_set() const noexcept;
		[[nodiscard]] const DescriptorCreateInfo& get_info() const noexcept;
		[[nodiscard]] VkDescriptorSetLayout get_layout() const noexcept;

		[[nodiscard]] static std::expected<DescriptorPool, vulkan::Error> create_descriptor_pool(LogicalDevice& device, DescriptorCreateInfo createInfo) noexcept;
		[[nodiscard]] static std::expected<DescriptorPool, vulkan::Error> create_descriptor_pool(LogicalDevice& device) noexcept;
	private:
		VkDescriptorSetLayout m_layout {VK_NULL_HANDLE};
		DescriptorCreateInfo m_createInfo;
	};
}