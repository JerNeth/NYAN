#ifndef VKSWAPCHAIN_H
#define VKSWAPCHAIN_H
#pragma once
//#include "VulkanIncludes.h"
//#include "Image.h"
//namespace vulkan {
//	class LogicalDevice;
//	struct SwapchainState {
//		VkPresentModeKHR presentMode;
//		VkSurfaceFormatKHR surfaceFormat;
//		VkExtent2D swapchainExtent;
//		VkSurfaceTransformFlagBitsKHR transform;
//		VkCompositeAlphaFlagBitsKHR compositeAlpha;
//		uint32_t minImageCount;
//	};
//	class Swapchain {
//	public:
//		Swapchain(LogicalDevice& parent, uint64_t id);
//		Swapchain(LogicalDevice& parent, uint64_t id, SwapchainState& state);
//		~Swapchain() noexcept;
//		Swapchain(Swapchain&) = delete;
//		Swapchain(Swapchain&&) noexcept;
//		Swapchain& operator=(Swapchain&) = delete;
//		Swapchain& operator=(Swapchain&&) = delete;
//		void recreate_swapchain();
//		uint32_t get_image_count() const;
//		void set_present_mode(VkPresentModeKHR mode) noexcept;
//		VkFormat get_swapchain_format() const noexcept;
//		const std::vector<std::unique_ptr<Image>>& get_swapchain_images() const noexcept;
//		uint32_t get_width() const;
//		uint32_t get_height() const;
//		VkExtent2D get_swapchain_extent() const;
//		//uint32_t aquire_next_image(VkSemaphore semaphore = VK_NULL_HANDLE, VkFence fence = VK_NULL_HANDLE, uint64_t timeout = UINT64_MAX);
//		void present_queue();
//		ImageInfo get_swapchain_image_info() const noexcept;
//		ImageView* get_swapchain_image_view(uint32_t imageIdx) const noexcept;
//		ImageView* get_swapchain_image_view() const noexcept;
//	private:
//		void update_extent();
//		void create_swapchain();
//		void destroy_swapchain() noexcept;
//		void create_images();
//		void create_image_views();
//
//		/// *******************************************************************
//		/// Member variables
//		/// *******************************************************************
//
//		LogicalDevice& r_device;
//		uint64_t m_id = ~0ull;
//		VkSwapchainKHR m_vkHandle = VK_NULL_HANDLE;
//		SwapchainState m_state{};
//		uint32_t imageIndex = 0;
//		uint32_t imageCount = 0;
//		bool m_dirtyState = true;
//		std::vector<std::unique_ptr<Image>> m_swapchainImages;
//		std::vector<std::unique_ptr<ImageView>> m_swapchainImageViews;
//	};
//}
#endif //VKSWAPCHAIN_H