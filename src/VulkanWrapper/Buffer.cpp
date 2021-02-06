#include "Buffer.h"
#include "LogicalDevice.h"

Vulkan::Buffer::~Buffer()
{
	r_device.queue_buffer_deletion( m_vkHandle, m_allocation );
}
