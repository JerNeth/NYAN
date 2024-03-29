#ifndef VKBUFFER_H
#define VKBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <Util>
#include "Allocator.h"
namespace vulkan {
	struct BufferInfo {
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		VkDeviceSize offset = 0;
		VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_UNKNOWN;
		static inline VkPipelineStageFlags buffer_usage_to_possible_stages(VkBufferUsageFlags usage)
		{
			VkPipelineStageFlags flags = 0;
			if (usage & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
				flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
			if (usage & (VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT))
				flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
			if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
				flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
			if (usage & (VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT |
				VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT))
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_VERTEX_SHADER_BIT |
				VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

			return flags;
		}

		static inline VkAccessFlags buffer_usage_to_possible_access(VkBufferUsageFlags usage)
		{
			VkAccessFlags flags = 0;
			if (usage & (VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT))
				flags |= VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			if (usage & VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
				flags |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
			if (usage & VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
				flags |= VK_ACCESS_INDEX_READ_BIT;
			if (usage & VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)
				flags |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
			if (usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)
				flags |= VK_ACCESS_UNIFORM_READ_BIT;
			if (usage & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
				flags |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

			return flags;
		}
	};
	class Buffer : public Utility::UIDC, public VulkanObject<VkBuffer>  {
	public:
		Buffer(LogicalDevice& device, VkBuffer buffer, VmaAllocation allocation, const BufferInfo& info);
		void resize(VkDeviceSize newSize, bool copyData = false);
		VkBufferUsageFlags get_usage()  const noexcept;
		VmaMemoryUsage get_memory_usage()  const noexcept;
		VkDeviceSize get_size() const noexcept;
		const BufferInfo& get_info() const noexcept;
		void swap_contents(Buffer& other) noexcept;
		//MappedMemoryHandle<uint8_t> map_data() const noexcept;
		void* map_data() noexcept;
		void unmap_data() noexcept;
		void flush(uint32_t offset = 0, uint32_t size = ~0u);
		void invalidate(uint32_t offset = 0, uint32_t size = ~0u);
		VkDeviceAddress get_address() const;
		~Buffer();
	private:
		VmaAllocation m_allocation;
		BufferInfo m_info;
		void* maped = nullptr;
	};
	using BufferHandle = Utility::ObjectHandle<Buffer, Utility::LinkedBucketList<Buffer>>;
}

#endif 