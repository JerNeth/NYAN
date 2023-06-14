#include "VulkanWrapper/Surface.hpp"
#include "VulkanWrapper/LogicalDevice.h"


vulkan::Surface::Surface(VkSurfaceKHR surface)
	: m_handle(surface)
{
	
}

vulkan::Surface::operator VkSurfaceKHR() const noexcept
{
	return m_handle;
}

const VkSurfaceKHR& vulkan::Surface::get_handle() const noexcept
{
	return m_handle;
}
