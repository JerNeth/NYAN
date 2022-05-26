#ifndef VKALLOCATOR_H
#define VKALLOCATOR_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include "Image.h"
#include <Util>
#include <unordered_map>

namespace vulkan {
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
		void invalidate(VmaAllocation allocation, uint32_t offset, uint32_t size);
		void free_allocation(VmaAllocation allocation) const noexcept {
			vmaFreeMemory(m_VmaHandle, allocation);
		}
	private:
		VmaAllocator m_VmaHandle = VK_NULL_HANDLE;
	};
	class Allocation {
	public:
		Allocation(LogicalDevice& device, VmaAllocation handle);
		Allocation(const Allocation& other) = delete;
		Allocation(Allocation&& other) noexcept;
		Allocation& operator=(const Allocation& other) = delete;
		Allocation& operator=(Allocation&& other);
		VmaAllocation get_handle() const noexcept {
			return m_VmaHandle;
		}
		VkDeviceSize get_size() const noexcept;
		VkDeviceSize get_offset() const noexcept;
		VkDeviceMemory get_memory() const noexcept;
		~Allocation();
	private:
		LogicalDevice& r_device;
		VmaAllocation m_VmaHandle = VK_NULL_HANDLE;
	};
	template<typename T>
	class MappedMemoryHandle {
	public:
		MappedMemoryHandle(Allocator* allocator, VmaAllocation allocation) : 
			m_allocation(allocation),
			m_allocator(allocator)
		{
			m_allocator->map_memory(m_allocation, reinterpret_cast<void**>(&m_ptr));
		}
		~MappedMemoryHandle() {
			if(m_allocator)
				m_allocator->unmap_memory(m_allocation);
		}
		MappedMemoryHandle(MappedMemoryHandle&) = delete;
		MappedMemoryHandle(MappedMemoryHandle&& other) noexcept : 
			m_ptr(other.m_ptr),
			m_allocation(other.m_allocation),
			m_allocator(other.m_allocator)
		{
			other.m_allocator = nullptr;
		}
		MappedMemoryHandle& operator=(const MappedMemoryHandle&) = delete;
		MappedMemoryHandle& operator=(MappedMemoryHandle&&) = delete;
		T* get() {
			return m_ptr;
		}
	private:
		T* m_ptr;
		VmaAllocation m_allocation;
		Allocator* m_allocator;
	};
	class AttachmentAllocator {
	public:
		AttachmentAllocator(LogicalDevice& parent);
		void clear() noexcept {
			m_attachmentIds.clear();
		}
		ImageView* request_attachment(uint32_t width, uint32_t height, VkFormat format, uint32_t index = 0, VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT, VkImageUsageFlags usage = 0);
	private:
		LogicalDevice& r_device;
		std::unordered_map<Utility::HashValue, ImageHandle> m_attachmentIds;
		std::shared_mutex m_mutex;
	};


}

#endif