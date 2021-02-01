#ifndef VKRENDERPASS_H
#define VKRENDERPASS_H
#pragma once
#include "VulkanIncludes.h"
#include "Image.h"

namespace Vulkan {
	class LogicalDevice;
	class Framebuffer;
	struct RenderpassCreateInfo {
		enum class OpFlags {
			DepthStencilClear = 0,
			DepthStencilLoad = 1,
			DepthStencilStore = 2,
			DepthStencilReadOnly = 3
		};
		enum class DepthStencil {
			None,
			Read,
			ReadWrite
		};
		//Last is depth
		std::bitset<MAX_ATTACHMENTS + 1> loadAttachments;
		std::bitset<MAX_ATTACHMENTS + 1> clearAttachments;
		std::bitset<MAX_ATTACHMENTS + 1> storeAttachments;
		std::array<ImageView*, MAX_ATTACHMENTS> colorAttachmentsViews{};
		ImageView* depthStencilAttachment = nullptr;
		uint32_t colorAttachmentsCount = 0;
		std::bitset<6> opFlags;

		
		struct SubpassCreateInfo {
			std::array<uint32_t, MAX_ATTACHMENTS> colorAttachments{};
			std::array<uint32_t, MAX_ATTACHMENTS> inputAttachments{};
			std::array<uint32_t, MAX_ATTACHMENTS> resolveAttachments{};
			DepthStencil depthStencil = DepthStencil::ReadWrite;
			uint32_t colorAttachmentsCount = 0;
			uint32_t inputAttachmentsCount = 0;
			uint32_t resolveAttachmentsCount = 0;
		};
		//TODO either dynamic array or other solution for magic number
		std::array<SubpassCreateInfo, 8> subpasses;
		uint32_t subpassCount = 0;
	};
	struct Subpass {
		std::array<VkAttachmentReference, MAX_ATTACHMENTS> colorAttachmentReferences;
		std::array<VkAttachmentReference, MAX_ATTACHMENTS> inputAttachmentReferences;
		VkAttachmentReference depthStencilAttachmentReference;
		uint32_t colorAttachmentCount = 0;
		uint32_t inputAttachmentCount = 0;
	};
	
	class Renderpass {
	public:
		Renderpass(LogicalDevice& parent, const RenderpassCreateInfo& createInfo);
		//Renderpass(LogicalDevice& parent, VkRenderPass renderPass);
		~Renderpass();
		bool has_depth_attachment(uint32_t subpass) const;
		uint32_t get_num_color_attachments(uint32_t subpass) const;
		VkRenderPass get_render_pass() const;
		const VkAttachmentReference& get_color_attachment(uint32_t subpass, uint32_t idx) const;
	private:
		LogicalDevice& r_device;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		std::array<VkFormat, MAX_ATTACHMENTS> colorAttachments{};
		VkFormat depthStencilAttachment = VK_FORMAT_UNDEFINED;
		std::vector<Subpass> subpasses;
	};
};
#endif