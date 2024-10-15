module;

#include <cassert>
#include <expected>
#include <utility>
#include <span>
#include <vector>

#include "volk.h"

module NYANVulkan;
import NYANData;
import NYANLog;

using namespace nyan::vulkan;


Surface::Surface(Surface&& other) noexcept :
	ptr_instance(std::exchange(other.ptr_instance, nullptr)),
	m_handle(std::exchange(other.m_handle, {}))
{
}
Surface& Surface::operator=(Surface&& other) noexcept
{
	if (this != std::addressof(other)) 
	{
		std::swap(ptr_instance, other.ptr_instance);
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

Surface::~Surface() noexcept
{
	if (m_handle.surface)
		vkDestroySurfaceKHR(ptr_instance->get_handle(), m_handle.surface, nullptr);
}

std::expected <PresentModes, Error> Surface::get_present_modes(const PhysicalDevice& device) const noexcept
{
	uint32_t count{ 0 };
	if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_handle(), m_handle.surface, &count, nullptr); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	assert(count <= 6);
	if (count > 6) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	nyan::StaticVector< VkPresentModeKHR, 6> presentModesVec(count);

	if (auto result = vkGetPhysicalDeviceSurfacePresentModesKHR(device.get_handle(), m_handle.surface, &count, presentModesVec.data()); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	PresentModes presentModes;
	for (auto p : presentModesVec)
		presentModes.set(convert_present_mode(p));
	return presentModes;
}

std::expected <PresentModes, Error> Surface::get_present_modes2(const PhysicalDevice& device) const noexcept
{
	uint32_t count{ 0 };
	if (auto result = vkGetPhysicalDeviceSurfacePresentModes2EXT(device.get_handle(), &m_handle, &count, nullptr); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	assert(count <= 6);
	if (count > 6)
		return std::unexpected{ VK_ERROR_UNKNOWN };

	nyan::StaticVector< VkPresentModeKHR, 6> presentModesVec(count);

	if (auto result = vkGetPhysicalDeviceSurfacePresentModes2EXT(device.get_handle(), &m_handle, &count, presentModesVec.data()); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	PresentModes presentModes;
	for (auto p : presentModesVec)
		presentModes.set(convert_present_mode(p));
	return presentModes;
}

std::expected<std::vector<Surface::SurfaceFormat>, Error> Surface::get_surface_formats(const PhysicalDevice& device) const noexcept
{
	uint32_t count{ 0 };
	if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_handle(), m_handle.surface, &count, nullptr); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	std::vector<VkSurfaceFormatKHR> surfaceFormats(count);

	if (auto result = vkGetPhysicalDeviceSurfaceFormatsKHR(device.get_handle(), m_handle.surface, &count, surfaceFormats.data()); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	std::vector<Surface::SurfaceFormat> retSurfaceFormats;

	retSurfaceFormats.reserve(surfaceFormats.size());
	for (auto& format : surfaceFormats)
		retSurfaceFormats.push_back(SurfaceFormat{ .format{static_cast<Format>(format.format)}, .colorSpace {convert_color_space(format.colorSpace)}});

	return retSurfaceFormats;
}



std::expected<Surface::Capabilities, Error> Surface::get_capabilites(const PhysicalDevice& device) const noexcept
{
	//VkSurfaceCapabilities2KHR capabilities{
	//   .sType {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_KHR},
	//   .pNext {nullptr}
	//};
	//VkSurfaceFullScreenExclusiveWin32InfoEXT fullscreenExclusiveWin32Info{
	//	.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_WIN32_INFO_EXT},
	//	.pNext {nullptr},
	//	.hmonitor {}
	//};
	//VkSurfaceFullScreenExclusiveInfoEXT fullscreenExclusiveInfo{
	//	.sType {VK_STRUCTURE_TYPE_SURFACE_FULL_SCREEN_EXCLUSIVE_INFO_EXT},
	//	.pNext {nullptr},
	//	.fullScreenExclusive {VK_FULL_SCREEN_EXCLUSIVE_DEFAULT_EXT }
	//};
	//VkSurfacePresentModeEXT presentModeInfo{
	//	.sType {VK_STRUCTURE_TYPE_SURFACE_PRESENT_MODE_EXT},
	//	.pNext {nullptr},
	//	.presentMode {}
	//};
	//VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo{
	//	.sType {VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR},
	//	.pNext {nullptr},
	//	.surface {surface}
	//};
	//if (auto result = vkGetPhysicalDeviceSurfaceCapabilities2KHR(m_vkHandle, &surfaceInfo, &capabilities); result != VK_SUCCESS) {
	//	if (result == VK_ERROR_SURFACE_LOST_KHR) {
	//		throw Utility::SurfaceLostException("vkGetPhysicalDeviceSurfaceCapabilities2KHR: Could not query surface formats");
	//	}
	//	else {
	//		throw Utility::VulkanException(result);
	//	}
	//}
	//return capabilities;
	VkSurfaceCapabilitiesKHR capabilities;

	if (auto result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device.get_handle(), m_handle.surface, &capabilities); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return Capabilities{
		.minImageCount {capabilities.minImageCount},
		.maxImageCount {capabilities.maxImageCount},
		.currentWidth {capabilities.currentExtent.width},
		.currentHeight {capabilities.currentExtent.height},
		.minWidth {capabilities.minImageExtent.width},
		.minHeight {capabilities.minImageExtent.height},
		.maxWidth {capabilities.maxImageExtent.width},
		.maxHeight {capabilities.maxImageExtent.height},
		.maxLayers {capabilities.maxImageArrayLayers},
		.supportedTransforms {static_cast<uint32_t>(capabilities.supportedTransforms)},
		.currentTransform {static_cast<uint32_t>(capabilities.currentTransform)},
		.supportedCompositeAlpha {static_cast<uint32_t>(capabilities.supportedCompositeAlpha)},
		.supportedUsage{convert_image_usage(capabilities.supportedUsageFlags)}
	};
}
#ifdef WIN32
std::expected<Surface, Error> Surface::create(const Instance& instance, void* hwnd, void* hinstance) noexcept
{
	static_assert(sizeof(hinstance) == sizeof(HINSTANCE));
	static_assert(sizeof(hwnd) == sizeof(HWND));
	VkWin32SurfaceCreateInfoKHR createInfo{
		.sType {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR},
		.pNext {nullptr},
		.flags {0},
		.hinstance {static_cast<HINSTANCE>(hinstance)},
		.hwnd {static_cast<HWND>(hwnd)},
	};

	VkSurfaceKHR handle{ VK_NULL_HANDLE };
	if (auto result = vkCreateWin32SurfaceKHR(instance.get_handle(), &createInfo, nullptr, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return Surface{instance, handle};
}
#endif


Surface::Surface(const Instance& instance, VkSurfaceKHR handle) noexcept :
	ptr_instance(&instance),
	m_handle(VkPhysicalDeviceSurfaceInfo2KHR { .sType{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR},.pNext{nullptr}, .surface{ handle } })
{
	assert(m_handle.surface != VK_NULL_HANDLE);
}
