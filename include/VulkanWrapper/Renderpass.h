#ifndef VKRENDERPASS_H
#define VKRENDERPASS_H
#pragma once
#include "VulkanIncludes.h"

#include "LogicalDevice.h"
namespace Vulkan {
	class Renderpass {
	public:
		bool has_depth_attachment(uint32_t subpass) const;
		uint32_t get_num_color_attachments(uint32_t subpass) const;
		const VkAttachmentReference& get_color_attachment(uint32_t subpass, uint32_t idx) const;
	private:
		VkAttachmentReference a;
		LogicalDevice& r_parent;
	};
};
#endif