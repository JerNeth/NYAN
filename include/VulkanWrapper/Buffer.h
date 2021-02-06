#ifndef VKBUFFER_H
#define VKBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Utility.h"
namespace Vulkan {
	class LogicalDevice;
	struct BufferInfo {
		VkDeviceSize size = 0;
		VkBufferUsageFlags usage = 0;
		VkDeviceSize offset = 0;
	};
	class Buffer : public Utility::UIDC {
	public:
		Buffer(LogicalDevice& device, VkBuffer buffer, VmaAllocation allocation) :
			r_device(device), 
			m_vkHandle(buffer),
			m_allocation(allocation)
		{

		}
		VkBuffer get_handle() const noexcept {
			return m_vkHandle;
		}
		~Buffer();
	private:
		LogicalDevice& r_device;
		VkBuffer m_vkHandle;
		VmaAllocation m_allocation;
		
	};
}

#endif 