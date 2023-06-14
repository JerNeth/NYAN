#include "VulkanWrapper/Queue.hpp"
#include "VulkanWrapper/LogicalDevice.h"
#include "Utility/Exceptions.h"

vulkan::Queue::Queue(LogicalDevice& device) :
	VulkanObject(device)
{
	//Required for vkQueueSubmit2
	assert(r_device.get_physical_device().get_vulkan13_features().synchronization2);
}

void vulkan::Queue::wait_idle() const
{
	assert(m_handle);
	if (const auto result = vkQueueWaitIdle(m_handle); result != VK_SUCCESS)
	{
		throw Utility::VulkanException(result);
	}
}

void vulkan::Queue::submit2(const std::span<VkSubmitInfo2> submits, const VkFence fence) const
{
	assert(m_handle);
	assert(submits.size() <= std::numeric_limits<uint32_t>::max());
	if (const auto result = vkQueueSubmit2(m_handle, static_cast<uint32_t>(submits.size()), submits.data(), fence); result != VK_SUCCESS)
	{
		throw Utility::VulkanException(result);
	}
}
