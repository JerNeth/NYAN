#ifndef VKFRAMEBUFFER_H
#define VKFRAMEBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Renderpass.h"
namespace Vulkan {
	class LogicalDevice;
	class Framebuffer {
	public:
		Framebuffer(LogicalDevice& parent, const RenderpassCreateInfo& renderpassInfo);
		Framebuffer(Framebuffer&) = delete;
		Framebuffer(Framebuffer&&) noexcept;
		Framebuffer& operator=(Framebuffer&) = delete;
		Framebuffer& operator=(Framebuffer&&) = delete;
		~Framebuffer() noexcept;
		VkFramebuffer get_handle() const noexcept{
			return m_vkHandle;
		}
	private:
		void init_dimensions(const RenderpassCreateInfo& renderpassInfo) noexcept;

		LogicalDevice& r_device;
		VkFramebuffer m_vkHandle = VK_NULL_HANDLE;
		uint32_t m_width = 0;
		uint32_t m_height = 0;
		uint32_t m_numAttachments = 0;
		bool imageless = false;
	};
}

#endif