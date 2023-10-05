#pragma once

#include <expected>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"

namespace vulkan
{
	class Allocator : public VulkanObject<::VmaAllocator>
	{
	public:

		Allocator(Allocator& other) = delete;
		Allocator& operator=(Allocator& other) = delete;

		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(Allocator&& other) noexcept;

		~Allocator();

		[[nodiscard]] std::expected<void*, vulkan::Error> map_memory(VmaAllocation allocation) const noexcept
		{
			void* data;
			if (const auto result = vmaMapMemory(m_handle, allocation, &data); result != VK_SUCCESS)
				return std::unexpected{vulkan::Error{result}};
			return data;
		}
		void unmap_memory(VmaAllocation allocation) const noexcept
		{
			vmaUnmapMemory(m_handle, allocation);
		}

		[[nodiscard]] std::expected<void, vulkan::Error> flush(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const noexcept
		{
			if (const auto result = vmaFlushAllocation(m_handle, allocation, offset, size); result != VK_SUCCESS)
				return std::unexpected{vulkan::Error{result}};
			return {};
		}

		[[nodiscard]] std::expected<void, vulkan::Error> invalidate(VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const noexcept
		{
			if (const auto result = vmaInvalidateAllocation(m_handle, allocation, offset, size); result != VK_SUCCESS)
				return std::unexpected{vulkan::Error{result}};
			return {};
		}

		void free_allocation(VmaAllocation allocation) const noexcept
		{
			vmaFreeMemory(m_handle, allocation);
		}

		static std::expected<vulkan::Allocator, vulkan::Error> create(const vulkan::Instance& instance, vulkan::LogicalDevice& device,
			VmaAllocatorCreateFlags createFlags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT |
			VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT) noexcept;
	private:
		Allocator(vulkan::LogicalDevice& device, ::VmaAllocator handle) noexcept;
	};
}