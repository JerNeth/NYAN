#include "Renderpass.h"

bool Vulkan::Renderpass::has_depth_attachment(uint32_t subpass) const
{
	return false;
}

uint32_t Vulkan::Renderpass::get_num_color_attachments(uint32_t subpass) const
{
	return 1;
}

const VkAttachmentReference& Vulkan::Renderpass::get_color_attachment(uint32_t subpass, uint32_t idx) const
{
	return a;
}
