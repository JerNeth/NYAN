#include "Swapchain.h"
#include "LogicalDevice.h"

Vulkan::Swapchain::Swapchain(LogicalDevice& parent, uint64_t id) : 
	r_device(parent),
	m_id(id)
{
	{
		auto surfaceFormats = r_device.r_instance.get_surface_formats();
		m_state.surfaceFormat = surfaceFormats[0];
		if (auto _surfaceFormat = std::find_if(surfaceFormats.cbegin(), surfaceFormats.cend(),
			[](auto format) {return format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR; });
			_surfaceFormat != surfaceFormats.cend())
			m_state.surfaceFormat = *_surfaceFormat;
	}
	{
		auto presentModes = r_device.r_instance.get_present_modes();
		m_state.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		if (std::find(presentModes.cbegin(), presentModes.cend(), VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.cend()) {
			m_state.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
		}
	}
	{
		auto capabilities = r_device.r_instance.get_surface_capabilites();
		m_state.minImageCount = capabilities.minImageCount + 1;
		if (capabilities.maxImageCount > 0 && m_state.minImageCount > capabilities.maxImageCount) {
			m_state.minImageCount = capabilities.maxImageCount;
		}
		m_state.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR) {
			m_state.compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		}
		if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) {
			m_state.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		}
		if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR) {
			m_state.compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
		}
		if (capabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) {
			m_state.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
		}
		m_state.transform = capabilities.currentTransform;
	}
	update_extent();
	create_swapchain();
	
}

Vulkan::Swapchain::Swapchain(LogicalDevice& parent, uint64_t id, SwapchainState& state) :
	r_device(parent),
	m_id(id),
	m_state(state)
{
	create_swapchain();
}

Vulkan::Swapchain::~Swapchain() noexcept
{
	destroy_swapchain();
}

Vulkan::Swapchain::Swapchain(Swapchain&& other) noexcept :
	r_device(other.r_device),
	m_id(other.m_id),
	m_vkHandle(other.m_vkHandle),
	m_state(other.m_state),
	imageIndex(other.imageIndex),
	imageCount(other.imageCount),
	m_dirtyState(other.m_dirtyState),
	m_swapchainImages(std::move(other.m_swapchainImages)),
	m_swapchainImageViews(std::move(other.m_swapchainImageViews))
{
	other.m_vkHandle = VK_NULL_HANDLE;
}

void Vulkan::Swapchain::recreate_swapchain()
{
	r_device.wait_idle();
	update_extent();
	create_swapchain();
	//r_device.create_swapchain(m_swapchainImageViews);
}


uint32_t Vulkan::Swapchain::get_image_count() const
{
	return imageCount;
}

void Vulkan::Swapchain::enable_vsync() noexcept
{
	if (m_state.presentMode != VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR) {
		m_state.presentMode = VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR;
		m_dirtyState = true;
	}
}

void Vulkan::Swapchain::disable_vsync() noexcept
{
	if (m_state.presentMode != VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR) {
		auto presentModes = r_device.r_instance.get_present_modes();
		if (std::find(presentModes.cbegin(), presentModes.cend(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.cend()) {
			m_state.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			m_dirtyState = true;
		}
	}
}

VkFormat Vulkan::Swapchain::get_swapchain_format() const
{
	return m_state.surfaceFormat.format;
}

const std::vector<std::unique_ptr<Vulkan::Image>>& Vulkan::Swapchain::get_swapchain_images() const
{
	return m_swapchainImages;
}

VkExtent2D Vulkan::Swapchain::get_swapchain_extent() const
{
	return m_state.swapchainExtent;
}

uint32_t Vulkan::Swapchain::get_width() const
{
	return m_state.swapchainExtent.width;
}

uint32_t Vulkan::Swapchain::get_height() const
{
	return m_state.swapchainExtent.height;
}

uint32_t Vulkan::Swapchain::aquire_next_image(VkSemaphore semaphore, VkFence fence, uint64_t timeout)
{
	VkResult result{};
	do {
		if (result = vkAcquireNextImageKHR(r_device.m_device, m_vkHandle, timeout, semaphore, fence, &imageIndex); result != VK_SUCCESS) {
			if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
				throw std::runtime_error("VK: could not acquire next image, out of host memory");
			}
			if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
				throw std::runtime_error("VK: could not acquire next image, out of device memory");
			}
			if (result == VK_ERROR_DEVICE_LOST) {
				throw std::runtime_error("VK: could not acquire next image, device lost");
			}
			if (result == VK_ERROR_OUT_OF_DATE_KHR) {
				recreate_swapchain();
			}
			if (result == VK_ERROR_SURFACE_LOST_KHR) {
				throw std::runtime_error("VK: could not acquire next image, surface lost");
			}
			if (result == VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT) {
				throw std::runtime_error("VK: could not acquire next image, fullscreen exclusive mode lost");
			}
			if (result == VK_SUBOPTIMAL_KHR) {
				recreate_swapchain();
			}
			else {
				throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
			}
		}
	} while (result != VK_SUCCESS);
	return imageIndex;
}

void Vulkan::Swapchain::present_to_queue(VkQueue queue, const VkSemaphore* semaphores, uint32_t semaphoreCount)
{
	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = semaphoreCount,
		.pWaitSemaphores = semaphores,
		.swapchainCount = 1,
		.pSwapchains = &m_vkHandle,
		.pImageIndices = &imageIndex,
		.pResults = NULL
	};

	if (auto result = vkQueuePresentKHR(queue, &presentInfo); result != VK_SUCCESS) {
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
			recreate_swapchain();
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}
Vulkan::ImageInfo Vulkan::Swapchain::get_swapchain_image_info() const noexcept
{
	ImageInfo info;
	info.format = get_swapchain_format();
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.isSwapchainImage = true;
	info.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	return info;
}
Vulkan::ImageView* Vulkan::Swapchain::get_swapchain_image_view(uint32_t imageIdx) const noexcept
{
	assert(imageIdx < imageCount);
	return m_swapchainImageViews[imageIdx].get();
}

Vulkan::ImageView* Vulkan::Swapchain::get_swapchain_image_view() const noexcept
{
	return get_swapchain_image_view(imageIndex);
}

void Vulkan::Swapchain::update_extent()
{
	auto capabilities = r_device.r_instance.get_surface_capabilites();
	m_state.swapchainExtent = capabilities.currentExtent;
	if (m_state.swapchainExtent.width == UINT32_MAX) {
		m_state.swapchainExtent = capabilities.maxImageExtent;
	}
}

void Vulkan::Swapchain::create_swapchain()
{

	//assert(m_vkHandle == VK_NULL_HANDLE);
	VkSwapchainKHR old = m_vkHandle;
	VkSwapchainCreateInfoKHR createInfo{
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = r_device.r_instance.get_surface(),
		.minImageCount = m_state.minImageCount,
		.imageFormat = m_state.surfaceFormat.format,
		.imageColorSpace = m_state.surfaceFormat.colorSpace,
		.imageExtent = m_state.swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
		.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.queueFamilyIndexCount = 0,
		.pQueueFamilyIndices = nullptr,
		.preTransform = m_state.transform,
		.compositeAlpha = m_state.compositeAlpha,
		.presentMode = m_state.presentMode,
		.clipped = VK_TRUE,
		.oldSwapchain = old
	};
	if (auto result = vkCreateSwapchainKHR(r_device.m_device, &createInfo, r_device.m_allocator, &m_vkHandle); result != VK_SUCCESS) {
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
	
	m_dirtyState = false;
	if (old != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(r_device.m_device, old, r_device.m_allocator);
	}
	create_images();
	create_image_views();
}

void Vulkan::Swapchain::destroy_swapchain() noexcept
{
	if (m_vkHandle != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(r_device.m_device, m_vkHandle, r_device.m_allocator);
		m_vkHandle = VK_NULL_HANDLE;
	}
}

void Vulkan::Swapchain::create_images()
{
	m_swapchainImages.clear();
	if (auto result = vkGetSwapchainImagesKHR(r_device.m_device, m_vkHandle, &imageCount, nullptr); result != VK_SUCCESS) {
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
	std::vector<VkImage> images(imageCount);
	if (auto result = vkGetSwapchainImagesKHR(r_device.m_device, m_vkHandle, &imageCount, images.data()); result != VK_SUCCESS) {
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
	ImageInfo info{
		.format = get_swapchain_format(),
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.isSwapchainImage = true,
		.width = get_width(),
		.height = get_height(),
		.depth = 1,
		.mipLevels = 1,
		.arrayLayers = 1,
		.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.type = VK_IMAGE_TYPE_2D,
		.layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
	};
	for (auto image : images)
		m_swapchainImages.emplace_back(new Image(r_device, image, info));
}

void Vulkan::Swapchain::create_image_views()
{
	m_swapchainImageViews.clear();

	for(auto& image : m_swapchainImages) {
		ImageViewCreateInfo createInfo{
			.image = image.get(),
			.format = get_swapchain_format(),
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		};
		m_swapchainImageViews.emplace_back(new ImageView(r_device, createInfo));
	}
}