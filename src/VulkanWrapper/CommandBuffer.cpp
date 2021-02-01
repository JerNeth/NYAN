#include "CommandBuffer.h"

Vulkan::CommandBuffer::CommandBuffer(LogicalDevice& parent, VkCommandBuffer handle) :
	r_device(parent),
	m_vkHandle(handle)
{
}