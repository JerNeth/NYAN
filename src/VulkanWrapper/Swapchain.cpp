#include "VulkanWrapper/Swapchain.hpp"

#include "Utility/Exceptions.h"

#include "VulkanWrapper/LogicalDevice.h"

vulkan::Swapchain::Swapchain(vulkan::LogicalDevice& device, VkSurfaceKHR surface) :
	VulkanObject(device),
	m_surface(surface)
{
}

vulkan::Swapchain::Swapchain(Swapchain&& other) noexcept :
	VulkanObject(other.r_device, other.m_handle),
	m_surface(other.m_surface)
{
	other.m_handle = VK_NULL_HANDLE;
}

vulkan::Swapchain::~Swapchain()
{
	if (m_handle)
		r_device.get_device().vkDestroySwapchainKHR( m_handle, r_device.get_allocator());
}

void vulkan::Swapchain::acquire_image()
{
	assert(m_handle);

	assert(m_acquiredImages.size() < m_swapchainImages.size());
	assert(m_acquiredImages.size() <= m_swapchainImages.size() - m_surfaceCapabilites.minImageCount);
	uint32_t index{ 0 };
	if(const auto result = r_device.get_device().vkAcquireNextImageKHR( m_handle, 0, VK_NULL_HANDLE, VK_NULL_HANDLE, &index); result != VK_SUCCESS)
	{
		if(result == VK_TIMEOUT)
		{
			//timeout != 0 || timeout != UINT64_MAX (infinite) no image acquired in timeout time
		}
		else if(result == VK_NOT_READY)
		{
			//If used with timeout = 0 and no image is available
		}
		else if(result == VK_SUBOPTIMAL_KHR)
		{
			//probably resized, swapchain recreation suggested
		}
		else if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			//No image acquired, presents will also fail with VK_ERROR_OUT_OF_DATE_KHR
			//Must create new swapchain
		}
		else if (result == VK_ERROR_SURFACE_LOST_KHR)
		{
			throw Utility::SurfaceLostException("vkAcquireNextImageKHR: lost surface");
		}
		else if (result == VK_ERROR_DEVICE_LOST)
		{
			throw Utility::DeviceLostException("vkAcquireNextImageKHR: lost device");
		}
		else if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT)
		{
			throw Utility::VulkanException(result);
		}
		else
		{
			throw Utility::VulkanException(result);
		}
	}
}

void vulkan::Swapchain::acquire_image_async()
{
}

void vulkan::Swapchain::create_swapchain()
{
	//VkSwapchainCreateInfoKHR createInfo{
	//	.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
	//	.surface = r_instance.get_surface(),
	//	.minImageCount = minImageCount,
	//	.imageFormat = surfaceFormat.format,
	//	.imageColorSpace = surfaceFormat.colorSpace,
	//	.imageExtent = m_swapchainExtent,
	//	.imageArrayLayers = 1,
	//	.imageUsage = m_usage,
	//	.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
	//	.queueFamilyIndexCount = 0,
	//	.pQueueFamilyIndices = nullptr,
	//	.preTransform = transform,
	//	.compositeAlpha = compositeAlpha,
	//	.presentMode = presentMode,
	//	.clipped = VK_TRUE,
	//	.oldSwapchain = old,
	//};
	//vkCreateSwapchainKHR(r_device, )
}
