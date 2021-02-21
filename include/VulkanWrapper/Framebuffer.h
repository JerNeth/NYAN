#ifndef VKFRAMEBUFFER_H
#define VKFRAMEBUFFER_H
#pragma once
#include "VulkanIncludes.h"
#include "Renderpass.h"
#include <Util>
namespace vulkan {
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
		VkExtent2D get_extent() const noexcept {
			return VkExtent2D { .width = m_width,.height = m_height };
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
	class FramebufferAllocator {
	public:
		FramebufferAllocator(LogicalDevice& device);
		void clear();
		Framebuffer* request_framebuffer(const RenderpassCreateInfo& info);
	private:
		LogicalDevice& r_device;
		std::unordered_map<Utility::HashValue, size_t> m_framebufferIds;
		Utility::LinkedBucketList<Framebuffer> m_framebufferStorage;
	};
}

#endif