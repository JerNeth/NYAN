#include "VulkanWrapper/WindowSystemInterface.h"
#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/Instance.h"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "Utility/Exceptions.h"


vulkan::WindowSystemInterface::WindowSystemInterface(LogicalDevice& device, Instance& instance, VkPresentModeKHR preferedPresentMode)
	:r_device(device),
	r_instance(instance),
	m_preferredPresentMode(preferedPresentMode)
{
	init_swapchain();
	r_device.init_swapchain(m_swapchainImages, m_swapchainExtent.width, m_swapchainExtent.height, m_format, m_usage);
}

vulkan::WindowSystemInterface::~WindowSystemInterface()
{
	if(m_vkHandle)
		r_device.get_device().vkDestroySwapchainKHR( m_vkHandle, r_device.get_allocator());
}

void vulkan::WindowSystemInterface::drain_swapchain()
{
	r_device.aquired_image(0, VK_NULL_HANDLE);
	r_device.wait_idle();
}

void vulkan::WindowSystemInterface::destroy_swapchain()
{
	drain_swapchain();
	if (m_vkHandle != VK_NULL_HANDLE)
		r_device.get_device().vkDestroySwapchainKHR( m_vkHandle, r_device.get_allocator());
	m_vkHandle = VK_NULL_HANDLE;
	m_swapchainImageAcquired = false;
}

void vulkan::WindowSystemInterface::update_swapchain()
{
	m_staleSwapchain = false;
	drain_swapchain();

	if(init_swapchain())//; Not sure if this was intentional
		r_device.init_swapchain(m_swapchainImages, m_swapchainExtent.width, m_swapchainExtent.height, m_format, m_usage);
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
		if (result = r_device.get_device().vkAcquireNextImageKHR( m_vkHandle, UINT64_MAX, semaphore, VK_NULL_HANDLE, &m_swapchainImageIndex); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
				update_swapchain();
			}
			else if (result != VK_NOT_READY) {
				throw Utility::VulkanException(result);
			}
		}
		else {
			m_swapchainImageAcquired = true;
			r_device.aquired_image(m_swapchainImageIndex, semaphore);
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
		.waitSemaphoreCount = semaphore != VK_NULL_HANDLE,
		.pWaitSemaphores = &semaphore,
		.swapchainCount = 1,
		.pSwapchains = &m_vkHandle,
		.pImageIndices = &m_swapchainImageIndex,
		.pResults = NULL
	};

	if (auto result = r_device.get_device().vkQueuePresentKHR(r_device.get_graphics_queue(), &presentInfo); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
			destroy_swapchain();
		}
		else {
			throw Utility::VulkanException(result);
		}
	}
}

void vulkan::WindowSystemInterface::set_vsync(bool enabled)
{
	m_vsyncEnabled = enabled;
}

bool vulkan::WindowSystemInterface::init_swapchain()
{

	auto surfaceFormats = r_device.get_physical_device().get_surface_formats2(r_instance.get_surface());

	std::vector<VkSurfaceFormatKHR> usableFormats;
	usableFormats.reserve(surfaceFormats.size());

	VkImageUsageFlags usage{ VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT };
	
	for (const auto& format : surfaceFormats) {
		if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
			r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
				VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
			usableFormats.push_back(format.surfaceFormat);
	}
	if (usableFormats.empty()) {
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
		for (const auto& format : surfaceFormats) {
			if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
					VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
				usableFormats.push_back(format.surfaceFormat);
		}
	}
	if (usableFormats.empty()) {
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		for (const auto& format : surfaceFormats) {
			if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
					VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
				usableFormats.push_back(format.surfaceFormat);
		}
	}
	if (usableFormats.empty()) {
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		for (const auto& format : surfaceFormats) {
			if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
					VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
				usableFormats.push_back(format.surfaceFormat);
		}
	}
	if (usableFormats.empty()) {
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		for (const auto& format : surfaceFormats) {
			if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
					VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
				usableFormats.push_back(format.surfaceFormat);
		}
	}
	if (usableFormats.empty()) {
		usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		for (const auto& format : surfaceFormats) {
			if (format.surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR &&
				r_device.get_physical_device().get_image_format_properties(format.surfaceFormat.format,
					VK_IMAGE_TYPE_2D, VK_IMAGE_TILING_OPTIMAL, usage, 0))
				usableFormats.push_back(format.surfaceFormat);
		}
	}
	m_usage = usage;
	VkSurfaceFormatKHR surfaceFormat{ usableFormats.front()};

	
	if (auto _surfaceFormat = std::find_if(usableFormats.cbegin(), usableFormats.cend(),
		[&](auto format) {return format.format == VK_FORMAT_B8G8R8A8_UNORM; });
		_surfaceFormat != usableFormats.cend())
		surfaceFormat = *_surfaceFormat;
	if (auto _surfaceFormat = std::find_if(usableFormats.cbegin(), usableFormats.cend(),
		[&](auto format) {return format.format == VK_FORMAT_A2B10G10R10_UNORM_PACK32; });
		_surfaceFormat != usableFormats.cend())
		surfaceFormat = *_surfaceFormat;
	if (auto _surfaceFormat = std::find_if(usableFormats.cbegin(), usableFormats.cend(),
		[&](auto format) {return format.format == m_preferredSwapchainFormat; });
		_surfaceFormat != usableFormats.cend())
		surfaceFormat = *_surfaceFormat;


	m_format = surfaceFormat.format;
	
	auto presentModes = r_device.get_physical_device().get_present_modes(r_instance.get_surface());
	auto presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
	if (std::find(presentModes.cbegin(), presentModes.cend(), m_preferredPresentMode) != presentModes.cend()) {
		presentMode = m_preferredPresentMode;
	}
	if (std::find(presentModes.cbegin(), presentModes.cend(), VK_PRESENT_MODE_IMMEDIATE_KHR) != presentModes.cend()) {
		presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
	}
	
	auto capabilities = r_device.get_physical_device().get_surface_capabilites2(r_instance.get_surface()).surfaceCapabilities;
	
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
		.imageUsage = m_usage, 
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = transform,
		.compositeAlpha = compositeAlpha,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = old,
	};
	if (auto result = r_device.get_device().vkCreateSwapchainKHR( &createInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}

	if (old != VK_NULL_HANDLE) {
		r_device.get_device().vkDestroySwapchainKHR( old, r_device.get_allocator());
	}
	uint32_t imageCount {0};
	if (auto result = r_device.get_device().vkGetSwapchainImagesKHR( m_vkHandle, &imageCount, nullptr); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
	m_swapchainImages.resize(imageCount);
	if (auto result = r_device.get_device().vkGetSwapchainImagesKHR( m_vkHandle, &imageCount, m_swapchainImages.data()); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}
	return true;
}
