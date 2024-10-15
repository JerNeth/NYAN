module;

//#include <algorithm>
//#include <cassert>
//#include <chrono>
//#include <expected>
//#include <utility>
//#include <span>

#include "volk.h"

module NYANVulkan;
import std;

import NYANData;
import NYANLog;

using namespace nyan::vulkan;

Swapchain::Swapchain(Swapchain&& other) noexcept :
	Object(*std::exchange(other.ptr_device, nullptr), std::exchange(other.m_handle, VK_NULL_HANDLE)),
	m_data(std::exchange(other.m_data, {}))
{

}

Swapchain& Swapchain::operator=(Swapchain&& other) noexcept
{
	if (this != std::addressof(other)) {
		std::swap(ptr_device, other.ptr_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_data, other.m_data);
	}
	return *this;
}

Swapchain::~Swapchain() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		ptr_device->vkDestroySwapchainKHR(m_handle);
}

std::expected<uint32_t, Error> Swapchain::aquire_image(std::chrono::duration<uint64_t, std::nano> waitTime) noexcept
{
	auto potentialIdx = (m_data.index + 1) % m_data.acquireSemaphores.size();
	//assert(potentialIdx < m_data.acquireSemaphores.size());
	for (uint32_t idx = 0; idx < m_data.acquireSemaphores.size(); ++idx) {
		if (m_data.acquireSemaphores[(potentialIdx + idx) % m_data.acquireSemaphores.size()].waited()) {
			potentialIdx = idx;
			break;
		}
		else if (idx == m_data.acquireSemaphores.size() - 1) [[unlikely]]
			return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	auto& semaphore = m_data.acquireSemaphores[potentialIdx];


	if (auto result = ptr_device->vkAcquireNextImageKHR(m_handle, waitTime.count(), semaphore.get_handle(), VK_NULL_HANDLE, &m_data.index); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	semaphore.m_timeline++;

	swap(semaphore, m_data.acquireSemaphores[m_data.index]);
	swap(m_data.presentSemaphores[potentialIdx], m_data.presentSemaphores[m_data.index]);

	return m_data.index;
}

std::expected<Swapchain, Error> Swapchain::create(LogicalDevice& device, Surface& surface, const Params& params) noexcept
{
	//Nvidia best practices
	//Rely on VK_PRESENT_MODE_FIFO_KHR or VK_PRESENT_MODE_MAILBOX_KHR(for VSync on).Noteworthy aspects :
	//VK_PRESENT_MODE_FIFO_KHR is preferred as it does not drop frames and lacks tearing.
	//VK_PRESENT_MODE_MAILBOX_KHR may offer lower latency, but frames might be dropped.
	//VK_PRESENT_MODE_FIFO_RELAXED_KHR is compelling when your application only occasionally lags behind the refresh rate, allowing tearing so that it can “catch back up”.
	//Rely on VK_PRESENT_MODE_IMMEDIATE_KHR for VSync off.
	//On Windows systems, use the VK_EXT_full_screen_exclusive extension to bypass compositing.
	//Handle both out - of - date and suboptimal swapchains to re - create stale swapchains when windows resize, for example.
	//For latency - sensitive applications, use the Vulkan Reflex SDK to minimize latency by completing game engine work just - in - time for rendering.
	const auto& physicalDevice = device.get_physical_device();

	auto surfaceFormatResult = surface.get_surface_formats(physicalDevice);
	if (!surfaceFormatResult) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };
	auto surfaceFormats = *surfaceFormatResult;

	auto capabilityResult = surface.get_capabilites(physicalDevice);
	if (!capabilityResult) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };
	auto capabilities = std::move(*capabilityResult);

	auto presentModeResult = surface.get_present_modes(physicalDevice);
	if (!presentModeResult) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };
	auto presentModes = std::move(*presentModeResult);

	/*auto surfaceFormats = r_device.get_physical_device().get_surface_formats2(r_instance.get_surface());

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
	VkSurfaceFormatKHR surfaceFormat{ usableFormats.front() };


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
	*/

	if (!std::any_of(surfaceFormats.begin(), surfaceFormats.end(), [&params] (auto f) { return f.format == params.format && f.colorSpace == params.colorSpace; })) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (!physicalDevice.image_format_supported(PhysicalDevice::FormatInfo{ .format{params.format}, .usage{params.usage} })) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (!presentModes.test(params.presentMode)) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if(!(capabilities.supportedCompositeAlpha & params.compositeAlpha)) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	VkSwapchainKHR handle{ VK_NULL_HANDLE };

	auto minImageCount = capabilities.minImageCount + 1;
	if (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount)
		minImageCount = std::min(capabilities.maxImageCount, maxNumImages);

	auto width = capabilities.currentWidth;
	auto height = capabilities.currentHeight;
	if (width == std::numeric_limits<uint32_t>::max() && height == std::numeric_limits<uint32_t>::max()) {
		width = capabilities.maxWidth;
		height = capabilities.maxHeight;
	}

	VkSwapchainCreateInfoKHR createInfo{
		.sType { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR },
		.surface { surface.get_handle()},
		.minImageCount { minImageCount},
		.imageFormat {static_cast<VkFormat>(params.format)},
		.imageColorSpace {static_cast<VkColorSpaceKHR>(convert_color_space(params.colorSpace)) },
		.imageExtent {.width {width}, .height{height}},
		.imageArrayLayers { 1 },
		.imageUsage {static_cast<VkImageUsageFlags>(convert_image_usage(params.usage))},
		.imageSharingMode {VK_SHARING_MODE_EXCLUSIVE},
		.queueFamilyIndexCount {0},
		.pQueueFamilyIndices {nullptr},
		.preTransform {static_cast<VkSurfaceTransformFlagBitsKHR>(capabilities.currentTransform)},
		.compositeAlpha {static_cast<VkCompositeAlphaFlagBitsKHR>(params.compositeAlpha)},
		.presentMode {static_cast<VkPresentModeKHR>(convert_present_mode(params.presentMode))},
		.clipped {params.clipped},
		.oldSwapchain {params.oldSwapchain ? params.oldSwapchain->get_handle() : VK_NULL_HANDLE},
	};

	if (auto result = device.get_device().vkCreateSwapchainKHR(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	uint32_t imageCount{ 0 };
	if (auto result = device.get_device().vkGetSwapchainImagesKHR(handle, &imageCount, nullptr); result != VK_SUCCESS) [[unlikely]] {
		device.get_device().vkDestroySwapchainKHR(handle);
		return std::unexpected{ result };
	}

	nyan::StaticVector<VkImage, maxNumImages> swapChainImages(imageCount);

	if (auto result = device.get_device().vkGetSwapchainImagesKHR(handle, &imageCount, swapChainImages.data()); result != VK_SUCCESS) [[unlikely]] {
		device.get_device().vkDestroySwapchainKHR(handle);
		return std::unexpected{ result };
	}


	decltype(Swapchain::Data::images) images;
	decltype(Swapchain::Data::acquireSemaphores) acquireSemaphores;
	decltype(Swapchain::Data::presentSemaphores) presentSemaphores;

	for (auto imageHandle : swapChainImages) {
		if (auto ret = images.try_push_back(SwapchainImage::create(device, SwapchainImage::SwapchainImageOptions{
				.handle{ imageHandle },
				.options{},
				.width{ static_cast<uint16_t>(createInfo.imageExtent.width)},
				.height{ static_cast<uint16_t>(createInfo.imageExtent.height) },
				.format{ params.format} ,
			})); !ret.has_value() || !ret.value()) [[unlikely]] {
			device.get_device().vkDestroySwapchainKHR(handle);
			return std::unexpected{ ret.error() };
		}
		if (auto ret = acquireSemaphores.try_push_back(BinarySemaphore::create(device)); !ret.has_value() || !ret.value()) [[unlikely]] {
			device.get_device().vkDestroySwapchainKHR(handle);
			return std::unexpected{ ret.error() };
		}
		if (auto ret = presentSemaphores.try_push_back(BinarySemaphore::create(device)); !ret.has_value() || !ret.value()) [[unlikely]] {
			device.get_device().vkDestroySwapchainKHR(handle);
			return std::unexpected{ ret.error() };
		}
	}

	return Swapchain{ device.get_device(), handle, Data{.width {createInfo.imageExtent.width}, .height{createInfo.imageExtent.height}, .hasMaintenance1 = false, .images{std::move(images)}, .acquireSemaphores{std::move(acquireSemaphores)}, .presentSemaphores{std::move(presentSemaphores)}} };
}

Swapchain::Swapchain(const LogicalDeviceWrapper& device, VkSwapchainKHR handle, Data data) noexcept :
	Object(device, handle),
	m_data(std::move(data))
{

}