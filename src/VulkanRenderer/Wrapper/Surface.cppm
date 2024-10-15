module;

//#include <bit>
//#include <expected>
//#include <span>
//#include <vector>

#include "magic_enum/magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:Surface;
import std;

import NYANData;

import :Common;
import :DeletionQueue;
import :PhysicalDevice;
import :Swapchain;
import :Error;
import :Object;

export namespace nyan::vulkan
{
	class Instance;
	class Surface
	{
	public:
		Surface(Surface&) = delete;
		Surface& operator=(Surface&) = delete;

		Surface(Surface&& other) noexcept;
		Surface& operator=(Surface&& other) noexcept;
		~Surface() noexcept;
		[[nodiscard]] VkSurfaceKHR get_handle() const noexcept {
			return m_handle.surface;
		}
		[[nodiscard("must handle potential error")]] std::expected < PresentModes, Error> get_present_modes(const PhysicalDevice& device) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected < PresentModes, Error> get_present_modes2(const PhysicalDevice& device) const noexcept;

		struct SurfaceFormat {
			Format format;
			ColorSpace colorSpace;
		};

		[[nodiscard("must handle potential error")]] std::expected < std::vector<SurfaceFormat>, Error> get_surface_formats(const PhysicalDevice& device) const noexcept;

		struct Capabilities {
			uint32_t minImageCount;
			uint32_t maxImageCount;
			uint32_t currentWidth;
			uint32_t currentHeight;
			uint32_t minWidth;
			uint32_t minHeight;
			uint32_t maxWidth;
			uint32_t maxHeight;
			uint32_t maxLayers;
			uint32_t supportedTransforms;
			uint32_t currentTransform;
			uint32_t supportedCompositeAlpha;
			ImageUsage supportedUsage;
		};

		[[nodiscard("must handle potential error")]] std::expected < Capabilities, Error> get_capabilites(const PhysicalDevice& device) const noexcept;
		
		[[nodiscard("must handle potential error")]] static std::expected<Surface, Error> create(const Instance& instance, void* hwind, void* hinstance) noexcept;
	private:
		Surface(const Instance& instance, VkSurfaceKHR handle) noexcept;

		const Instance* ptr_instance{ nullptr };
		VkPhysicalDeviceSurfaceInfo2KHR m_handle{ .sType{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR},.pNext{nullptr}, .surface{ VK_NULL_HANDLE } };


	};
}