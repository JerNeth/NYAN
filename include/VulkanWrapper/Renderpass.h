#ifndef VKRENDERPASS_H
#define VKRENDERPASS_H
#pragma once
#include "VulkanIncludes.h"
#include "Utility.h"
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
		std::array<VkClearColorValue, MAX_ATTACHMENTS> clearColors;
		VkClearDepthStencilValue clearDepthStencil;

		
		struct SubpassCreateInfo {
			std::array<uint32_t, MAX_ATTACHMENTS> colorAttachments{};
			std::array<uint32_t, MAX_ATTACHMENTS> inputAttachments{};
			std::array<uint32_t, MAX_ATTACHMENTS> resolveAttachments{};
			DepthStencil depthStencil = DepthStencil::ReadWrite;
			uint32_t colorAttachmentsCount = 0;
			uint32_t inputAttachmentsCount = 0;
			uint32_t resolveAttachmentsCount = 0;
		};
		VkRect2D renderArea;
		//TODO either dynamic array or other solution for magic number
		std::array<SubpassCreateInfo, 8> subpasses;
		uint32_t subpassCount = 0;
		std::pair<Utility::HashValue, Utility::HashValue> get_hash() const noexcept {
			Utility::Hasher hasher;
			//std::bitset<MAX_ATTACHMENTS + 1> optimalLayouts;

			hasher(colorAttachmentsCount);
			for (uint32_t i = 0; i < colorAttachmentsCount; i++) {
				auto attachment = colorAttachmentsViews[i];
				hasher(attachment->get_format());
				//Ignore Optimal Layout for now
				//if (attachment->get_image()->get_info().layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				//	optimalLayouts.set(i);

				hasher(attachment->get_image()->get_info().layout);

			}
			if (depthStencilAttachment) {
				//Ignore Optimal Layout for now
				//if (info.depthStencilAttachment->get_image()->get_info().layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
				//	optimalLayouts.set(info.colorAttachmentsCount);
				hasher(depthStencilAttachment->get_format());
			}
			else {
				hasher(VK_FORMAT_UNDEFINED);
			}


			//Ignore Multiview for now
			hasher(subpassCount);
			for (uint32_t i = 0; i < subpassCount; i++) {
				const auto& subpass = subpasses[i];
				hasher(subpass.colorAttachmentsCount);
				hasher(subpass.inputAttachmentsCount);
				hasher(subpass.resolveAttachmentsCount);
				hasher(subpass.depthStencil);
				for (uint32_t j = 0; j < subpass.colorAttachmentsCount; j++)
					hasher(subpass.colorAttachments[j]);
				for (uint32_t j = 0; j < subpass.inputAttachmentsCount; j++)
					hasher(subpass.inputAttachments[j]);
				for (uint32_t j = 0; j < subpass.resolveAttachmentsCount; j++)
					hasher(subpass.resolveAttachments[j]);
			}
			Utility::HashValue compatibleHash = hasher();
			hasher(opFlags.to_ulong());
			hasher(loadAttachments.to_ulong());
			hasher(clearAttachments.to_ulong());
			hasher(storeAttachments.to_ulong());
			return {compatibleHash, hasher()};
		}
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
		const VkAttachmentReference& get_color_attachment(uint32_t idx, uint32_t subpass = 0) const;
		Utility::HashValue get_compatible_hash() const noexcept {
			return m_compatibleHashValue;
		}
		Utility::HashValue get_hash() const noexcept {
			return m_hashValue;
		}
	private:
		LogicalDevice& r_device;
		VkRenderPass m_renderPass = VK_NULL_HANDLE;

		std::array<VkFormat, MAX_ATTACHMENTS> colorAttachments{};
		VkFormat depthStencilAttachment = VK_FORMAT_UNDEFINED;
		std::vector<Subpass> m_subpasses;
		Utility::HashValue m_compatibleHashValue, m_hashValue;
	};
};
#endif