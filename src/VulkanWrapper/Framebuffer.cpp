#include "Framebuffer.h"
#include "LogicalDevice.h"

Vulkan::Framebuffer::Framebuffer(LogicalDevice& parent, const RenderpassCreateInfo& renderpassInfo) :
    r_device(parent),
	m_numAttachments(renderpassInfo.colorAttachmentsCount + (renderpassInfo.usingDepth ? 1 : 0))
{
	auto rp = r_device.request_compatible_render_pass(renderpassInfo);
	init_dimensions(renderpassInfo);
	std::array<VkImageView, MAX_ATTACHMENTS + 1> attachments;
	
	VkFramebufferCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		.renderPass = rp->get_render_pass(),
		.attachmentCount = m_numAttachments,
		.width = m_width,
		.height = m_height,
		.layers = 1
	};
	
	std::array < VkFramebufferAttachmentImageInfo, MAX_ATTACHMENTS + 1> imageInfos;
	std::array < std::array<VkFormat, 2>, MAX_ATTACHMENTS +1> formats;
	VkFramebufferAttachmentsCreateInfoKHR attachmentsCreateInfo{
		.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO_KHR,
		.attachmentImageInfoCount = m_numAttachments,
		.pAttachmentImageInfos = imageInfos.data()
	};
	if (imageless) {
		createInfo.pNext = &attachmentsCreateInfo;
		createInfo.flags = VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT_KHR;
		for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
			auto &info = imageInfos[i];
			info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
			auto mip = renderpassInfo.colorAttachmentsViews[i]->get_base_mip_level();
			info.width = renderpassInfo.colorAttachmentsViews[i]->get_image()->get_width(mip);
			info.height = renderpassInfo.colorAttachmentsViews[i]->get_image()->get_height(mip);
			info.usage = renderpassInfo.colorAttachmentsViews[i]->get_image()->get_usage();
			info.viewFormatCount = ImageInfo::compute_view_formats(renderpassInfo.colorAttachmentsViews[i]->get_image()->get_info(), formats[i]);
			info.pViewFormats = formats[i].data();
		}
	}
	else {
		for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
			attachments[i] = renderpassInfo.colorAttachmentsViews[i]->get_image_view();
		}
		if (renderpassInfo.usingDepth)
			attachments[renderpassInfo.colorAttachmentsCount] = renderpassInfo.depthStencilAttachment->get_image_view();
		createInfo.pAttachments = attachments.data();
	}
	if (auto result = vkCreateFramebuffer(r_device.get_device(), &createInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create framebuffer, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create framebuffer, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}

Vulkan::Framebuffer::Framebuffer(Framebuffer&& other) noexcept: 
	r_device(other.r_device),
	m_vkHandle(other.m_vkHandle),
	m_width(other.m_width),
	m_height(other.m_height),
	m_numAttachments(other.m_numAttachments)
{
	other.m_vkHandle = VK_NULL_HANDLE;
}

Vulkan::Framebuffer::~Framebuffer() noexcept
{
    if(m_vkHandle != VK_NULL_HANDLE) {
        vkDestroyFramebuffer(r_device.get_device(), m_vkHandle, r_device.get_allocator());
    }
}

void Vulkan::Framebuffer::init_dimensions(const RenderpassCreateInfo& renderpassInfo) noexcept
{
	m_width = ~0;
	m_height = ~0;
	if (renderpassInfo.colorAttachmentsCount) {
		for (uint32_t i = 0; i < renderpassInfo.colorAttachmentsCount; i++) {
			auto mip = renderpassInfo.colorAttachmentsViews[0]->get_base_mip_level();
			m_width = Math::min(m_width, renderpassInfo.colorAttachmentsViews[0]->get_image()->get_width(mip));
			m_height = Math::min(m_height, renderpassInfo.colorAttachmentsViews[0]->get_image()->get_height(mip));
		}
	}
	
	if (renderpassInfo.usingDepth) {
		auto mip = renderpassInfo.depthStencilAttachment->get_base_mip_level();
		m_width = Math::min(m_width, renderpassInfo.depthStencilAttachment->get_image()->get_width(mip));
		m_height = Math::min(m_height, renderpassInfo.depthStencilAttachment->get_image()->get_height(mip));
	}
}
