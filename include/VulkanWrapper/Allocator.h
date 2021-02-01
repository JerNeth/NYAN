#ifndef VKALLOCATOR_H
#define VKALLOCATOR_H
#pragma once
#include "VulkanIncludes.h"
#include "Image.h"
#include "Utility.h"

namespace Vulkan {
	class LogicalDevice;
	class Allocator {
	public:
		Allocator(VmaAllocator handle);
		Allocator(Allocator&) = delete;
		Allocator(Allocator&&) noexcept;
		Allocator& operator=(Allocator&) = delete;
		Allocator& operator=(Allocator&&) = delete;
		~Allocator() noexcept;
		VmaAllocator get_handle() const noexcept {
			return m_VmaHandle;
		}
		void map_memory(VmaAllocation allocation, void** data);
		void unmap_memory(VmaAllocation allocation);
		void destroy_buffer(VkBuffer buffer, VmaAllocation allocation);
		void flush(VmaAllocation allocation, uint32_t offset, uint32_t size);
	private:
		VmaAllocator m_VmaHandle = VK_NULL_HANDLE;
	};

	class AttachmentAllocator {
	public:
		AttachmentAllocator(LogicalDevice& parent);
		ImageView* request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index = 0, uint32_t samples = 1, uint32_t layers = 1);
	private:
		LogicalDevice& r_device;
		std::unordered_map<Utility::HashValue, size_t> m_attachmentIds;
		Utility::LinkedBucketList<ImageView> m_imageViewStorage;
		Utility::LinkedBucketList<Image> m_imageStorage;
		std::shared_mutex m_mutex;
	};
}

#endif