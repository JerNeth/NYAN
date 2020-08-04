#ifndef VKLOGICALDEVICE_H
#define VKLOGICALDEVICE_H
#pragma once
#include "VkWrapper.h"
namespace vk {
	class Instance;
	constexpr size_t MAX_FRAMES_IN_FLIGHT = 3;
	class LogicalDevice {
	public:
		LogicalDevice(const Instance& parentInstance, VkDevice device, uint32_t graphicsQueueFamilyIndex);
		~LogicalDevice();
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&) = delete;
		void create_swapchain();
		void create_graphics_pipeline();
		void create_image_views();
		void create_render_pass();
		void create_framebuffers();
		void create_command_pool();
		void create_command_buffers();
		void create_sync_objects();
		void draw_frame();
		void wait_idle();
	private:
		VkShaderModule create_shader_module(const std::vector<char>& shaderCode);

		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		

		VkDevice m_device;
		const Instance& m_parentInstance;
		VkAllocationCallbacks* m_allocator = NULL;
		const uint32_t m_graphicsFamilyQueueIndex;

		VkSwapchainKHR m_swapChain;
		std::vector<VkImage> m_swapChainImages;
		VkExtent2D m_swapChainExtent;
		VkFormat m_swapChainImageFormat;
		std::vector<VkImageView> m_swapChainImageViews;
		VkPipelineLayout m_pipelineLayout;
		VkRenderPass m_renderPass;
		VkPipeline m_graphicsPipeline;
		std::vector<VkFramebuffer> m_swapChainFramebuffers;
		VkCommandPool m_commandPool;
		std::vector<VkCommandBuffer> m_commandBuffers;
		std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
		std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
		std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFences;
		std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_imagesInFlight;
		size_t m_currentFrame = 0;

		VkQueue m_graphicsQueue;

		
	};
}
#endif // VKLOGICALDEVICE_H