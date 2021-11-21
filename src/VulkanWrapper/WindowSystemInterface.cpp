#include "WindowSystemInterface.h"
#include "LogicalDevice.h"
#include "Instance.h"

vulkan::WindowSystemInterface::WindowSystemInterface(LogicalDevice& device, Instance& instance, VkPresentModeKHR preferedPresentMode)
	:r_device(device),
	r_instance(instance),
	m_preferredPresentMode(preferedPresentMode)
{
	init_swapchain();
	r_device.init_swapchain(m_swapchainImages, m_swapchainExtent.width, m_swapchainExtent.height, m_format);
}

vulkan::WindowSystemInterface::~WindowSystemInterface()
{
	if(m_vkHandle)
		vkDestroySwapchainKHR(r_device.get_device(), m_vkHandle, r_device.get_allocator());
}

void vulkan::WindowSystemInterface::drain_swapchain()
{
	r_device.set_acquire_semaphore(0, VK_NULL_HANDLE);
	r_device.wait_idle();
}

void vulkan::WindowSystemInterface::destroy_swapchain()
{
	drain_swapchain();
	if (m_vkHandle != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(r_device.get_device(), m_vkHandle, r_device.get_allocator());
	m_vkHandle = VK_NULL_HANDLE;
	m_swapchainImageAcquired = false;
}

void vulkan::WindowSystemInterface::update_swapchain()
{
	m_staleSwapchain = false;
	drain_swapchain();

	if(init_swapchain())//; Not sure if this was intentional
		r_device.init_swapchain(m_swapchainImages, m_swapchainExtent.width, m_swapchainExtent.height, m_format);
}

void vulkan::WindowSystemInterface::begin_frame()
{
	r_device.next_frame();
	if (m_staleSwapchain || m_vkHandle == VK_NULL_HANDLE)
		update_swapchain();
	if (m_swapchainImageAcquired)
		return;
	VkResult result{};
	do {
		auto semaphore = r_device.request_semaphore();
		if (result = vkAcquireNextImageKHR(r_device.get_device(), m_vkHandle, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_swapchainImageIndex); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not acquire next image, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not acquire next image, out of device memory");
			}
			if (result == VK_ERROR_DEVICE_LOST) {
				throw std::runtime_error("VK: could not acquire next image, device lost");
			}
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				update_swapchain();
			}
			if (result == VK_ERROR_SURFACE_LOST_KHR) {
				throw std::runtime_error("VK: could not acquire next image, surface lost");
			}
			if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
				throw std::runtime_error("VK: could not acquire next image, fullscreen exclusive mode lost");
			}
			else if (result != VK_NOT_READY) {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
		else {
			m_swapchainImageAcquired = true;
			r_device.set_acquire_semaphore(m_swapchainImageIndex, semaphore);
		}
	} while (result != VK_SUCCESS);
}

void vulkan::WindowSystemInterface::end_frame()
{
	r_device.end_frame();
	if (!r_device.swapchain_touched())
		return;
	if (!m_swapchainImageAcquired)
		return;
	m_swapchainImageAcquired = false;
	auto semaphore = r_device.get_present_semaphore();
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &semaphore,
		.swapchainCount = 1,
		.pSwapchains = &m_vkHandle,
		.pImageIndices = &m_swapchainImageIndex,
		.pResults = NULL
	};

	if (auto result = vkQueuePresentKHR(r_device.get_graphics_queue(), &presentInfo); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not present, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not present, out of device memory");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not present, device lost");
		}
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not present, surface lost");
		}
		if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
			throw std::runtime_error("VK: could not present, fullscreen exclusive mode lost");
		}
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			destroy_swapchain();
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

void vulkan::WindowSystemInterface::set_vsync(bool enabled)
{
	m_vsyncEnabled = enabled;
}

bool vulkan::WindowSystemInterface::init_swapchain()
{

	auto surfaceFormats = r_instance.get_surface_formats();
	auto surfaceFormat = surfaceFormats[0];
	if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
		[&](auto format) {return format.format == m_preferredSwapchainFormat && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; });
		_surfaceFormat != surfaceFormats.cend())
		surfaceFormat = *_surfaceFormat;
	//if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
	//	[](auto format) {return format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; });
	//	_surfaceFormat != surfaceFormats.cend())
	//	surfaceFormat = *_surfaceFormat;
	//if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
	//	[](auto format) {return format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32 && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; });
	//	_surfaceFormat != surfaceFormats.cend())
	//	surfaceFormat = *_surfaceFormat;

	m_format = surfaceFormat.format;
	
	auto presentModes = r_instance.get_present_modes();
	auto presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.cbegin(), presentModes.cend(), m_preferredPresentMode) != presentModes.cend()) {
		presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	}
	
	
	auto capabilities = r_instance.get_surface_capabilites();
	
	auto minImageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) {
		minImageCount = capabilities.maxImageCount;
	}
	auto compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
		compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
	}
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
		compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
	}
	if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
		compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	auto transform = capabilities.currentTransform;
	m_swapchainExtent = capabilities.currentExtent;
	if (m_swapchainExtent.width == UINT32_MAX) {
		m_swapchainExtent = capabilities.maxImageExtent;
	}
	VkSwapchainKHR old = m_vkHandle;
	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = r_instance.get_surface(),
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = m_swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = transform,
		.compositeAlpha = compositeAlpha,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = old
	};
	if (auto result = vkCreateSwapchainKHR(r_device.get_device(), &createInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		if (result == VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) {
			throw std::runtime_error("VK: could not create swapchain, native window in use");
		}
		if (result == VK_ERROR_INITIALIZATION_FAILED) {
			throw std::runtime_error("VK: could not create swapchain, initialization failed");
		}
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			throw std::runtime_error("VK: could not create swapchain, surface lost");
		}
		if (result == VK_ERROR_DEVICE_LOST) {
			throw std::runtime_error("VK: could not create swapchain, device lost");
		}
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create swapchain, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}

	if (old != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(r_device.get_device(), old, r_device.get_allocator());
	}
	uint32_t imageCount;
	if (auto result = vkGetSwapchainImagesKHR(r_device.get_device(), m_vkHandle, &imageCount, nullptr); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not get swapchain image count, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not get swapchain image count, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	m_swapchainImages.resize(imageCount);
	if (auto result = vkGetSwapchainImagesKHR(r_device.get_device(), m_vkHandle, &imageCount, m_swapchainImages.data()); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not get swapchain images, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not get swapchain images, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	return true;
}
