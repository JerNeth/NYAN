module;

//#include <bit>
//#include <chrono>
//#include <expected>
//#include <span>

#include "magic_enum.hpp"

#include "volk.h"

export module NYANVulkan:Swapchain;
import std;

import NYANData;

import :DeletionQueue;
import :Error;
import :Image;
import :Object;
import :Queue;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class Surface;
	//Create dependend on PhysicalDevice and SurfaceKHR
	//Present (multiple) swapchain(s) on a (present capable) queue
	class Swapchain : public Object<VkSwapchainKHR>
	{
	private:

		static constexpr uint32_t maxNumImages = 4;
		struct Data {
			uint32_t index{ 0 };
			uint32_t width{ 0 };
			uint32_t height{ 0 };
			bool hasMaintenance1{ 0 };
			nyan::StaticVector<SwapchainImage, maxNumImages> images;
			nyan::StaticVector<BinarySemaphore, maxNumImages> acquireSemaphores;
			nyan::StaticVector<BinarySemaphore, maxNumImages> presentSemaphores;
		};
	public:
		Swapchain(Swapchain&) = delete;
		Swapchain& operator=(Swapchain&) = delete;

		Swapchain(Swapchain&& other) noexcept;
		Swapchain& operator=(Swapchain&& other) noexcept;
		~Swapchain() noexcept;

		[[nodiscard]] uint32_t get_index() const noexcept {
			return m_data.index;
		}

		[[nodiscard]] uint32_t get_width() const noexcept {
			return m_data.width;
		}

		[[nodiscard]] uint32_t get_height() const noexcept {
			return m_data.height;
		}
		[[nodiscard]] std::span<const SwapchainImage> get_images() const noexcept {
			return { m_data.images.data(), m_data.images.size() };
		}
		[[nodiscard]] std::span<const BinarySemaphore> get_acquire_semaphores() const noexcept {
			return { m_data.acquireSemaphores.data(), m_data.acquireSemaphores.size() };
		}
		[[nodiscard]] std::span<const BinarySemaphore> get_present_semaphores() const noexcept {
			return { m_data.presentSemaphores.data(), m_data.presentSemaphores.size() };
		}
		[[nodiscard]] std::span<BinarySemaphore> get_acquire_semaphores() noexcept {
			return { m_data.acquireSemaphores.data(), m_data.acquireSemaphores.size() };
		}
		[[nodiscard]] std::span<BinarySemaphore> get_present_semaphores() noexcept {
			return { m_data.presentSemaphores.data(), m_data.presentSemaphores.size() };
		}

		[[nodiscard("must handle potential error")]] std::expected<uint32_t, Error> aquire_image(std::chrono::duration<uint64_t, std::nano> waitTime = std::chrono::duration<uint64_t, std::nano>{ std::numeric_limits<uint64_t>::max() }) noexcept;

		struct Params {
			Format format;
			ColorSpace colorSpace;
			ImageUsage usage{ ImageUsageFlags::ColorAttachment};
			uint32_t compositeAlpha{static_cast<uint32_t>(VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)};
			PresentMode presentMode{PresentMode::Immediate};
			bool clipped{ true };
			Swapchain* oldSwapchain{ nullptr };
		};

		[[nodiscard("must handle potential error")]] static std::expected<Swapchain, Error> create(LogicalDevice& device, Surface& surface, const Params& params) noexcept;
	private:
		Swapchain(const LogicalDeviceWrapper& device, VkSwapchainKHR handle, Data data) noexcept;

		Data m_data;
	};
}