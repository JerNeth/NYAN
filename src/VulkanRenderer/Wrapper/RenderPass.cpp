module;

//#include <cassert>
//#include <expected>
//#include <span>
//#include <utility>

#include "volk.h"
#include "vk_mem_alloc.h"


module NYANVulkan;
import std;

import NYANData;
import NYANLog;

using namespace nyan::vulkan;
using namespace nyan;


std::expected<RenderPass, Error> RenderPass::create(LogicalDevice& device) noexcept
{
	VkAttachmentDescription attachment{
	.flags { 0 }, //VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT //VkAttachmentDescriptionFlags   
	.format {}, //param										//VkFormat                       
	.samples {}, //param									//VkSampleCountFlagBits          
	.loadOp {}, //deduce									//VkAttachmentLoadOp             
	.storeOp {}, //deduce									//VkAttachmentStoreOp            
	.stencilLoadOp {}, //deduce								//VkAttachmentLoadOp             
	.stencilStoreOp {}, //deduce							//VkAttachmentStoreOp            
	.initialLayout {}, //maybe deduce, maybe param			//VkImageLayout                  
	.finalLayout {}, //maybe deduce, maybe param			//VkImageLayout                  
	};
	VkSubpassDescription subpasses{
	.flags  {0},											//VkSubpassDescriptionFlags       
	.pipelineBindPoint {VK_PIPELINE_BIND_POINT_GRAPHICS },	//VkPipelineBindPoint             
	.inputAttachmentCount {},								//uint32_t                        
	.pInputAttachments {},		//param						//const VkAttachmentReference*    
	.colorAttachmentCount {},								//uint32_t                        
	.pColorAttachments {},		//param						//const VkAttachmentReference*    
	.pResolveAttachments {},								//const VkAttachmentReference*    
	.pDepthStencilAttachment {},//param						//const VkAttachmentReference*    
	.preserveAttachmentCount {},							//uint32_t                        
	.pPreserveAttachments {},	//have to deduce			//const uint32_t*                 
	};
	VkSubpassDependency dependencies{
	.srcSubpass {},		//param								//uint32_t                
	.dstSubpass {},		//param								//uint32_t                
	.srcStageMask {},	//prob deduce						//VkPipelineStageFlags    
	.dstStageMask {},	//prob deduce						//VkPipelineStageFlags    
	.srcAccessMask {},	//prob deduce						//VkAccessFlags           
	.dstAccessMask {},	//prob deduce						//VkAccessFlags           
	.dependencyFlags {0},									//VkDependencyFlags, maybe feedback loop in the future
	};
	struct Params {
		struct Attachment {
			enum class LoadOp : uint8_t {
				Load,
				Clear,
				DontCare
			};
			enum class StoreOp : uint8_t {
				Store,
				DontCare
			};
			LoadOp load;
			StoreOp store;
			RenderFormat format;
			Samples samples;
			Layout initialLayout;
			Layout finalLayout;
		};
		std::array<Attachment, 1> attachments;
		uint8_t attachmentCount;
		struct Subpass {
			std::array<uint8_t, 1> inputs;
			std::array<uint8_t, 1> colors;
			std::array<uint8_t, 1> resolves;
			uint8_t inputCount; //In practice between 60 and 128 (later most common) (maxFragmentInputComponents)
			uint8_t colorCount; //In practice 8 is common device limit (maxFragmentOutputAttachments)
			enum class DepthMode : uint8_t {
				readWrite,
				read,
			} depthStencil;
		};
		std::array<Subpass, 1> subpasses;
	};
	VkRenderPassCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.attachmentCount {0},
		.pAttachments {nullptr},
		.subpassCount {0},
		.pSubpasses {nullptr},
		.dependencyCount {0},
		.pDependencies {nullptr}
	};

	VkRenderPass handle{ VK_NULL_HANDLE };

	if (auto result = device.get_device().vkCreateRenderPass(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]] 
		return std::unexpected{ result };

	::assert(false, "TODO");

	return RenderPass{ device.get_device(), device.get_deletion_queue(), handle, {} };
}

RenderPass::RenderPass(RenderPass&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	r_deletionQueue(other.r_deletionQueue), 
	m_data(std::move(other.m_data))
{

}

RenderPass& RenderPass::operator=(RenderPass&& other) noexcept
{
	::assert(ptr_device == other.ptr_device);
	::assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	if (std::addressof(other) != this) {
		std::swap(m_handle, other.m_handle);
		std::swap(m_data, other.m_data);
	}
	return *this;
}

RenderPass::~RenderPass() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_handle);
}

uint32_t RenderPass::get_num_color_attachments(uint32_t subpass) const noexcept
{
	return m_data.numColorAttachments[subpass];
}

std::span<const ClearValue> RenderPass::get_clear_values() const noexcept
{
	return {m_data.clearValues.data(), m_data.clearValues.size()};
}

RenderPass::RenderPass(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkRenderPass handle, Data data) noexcept :
	Object(device, handle),
	r_deletionQueue(deletionQueue),
	m_data(std::move(data))
{
	::assert(m_handle != VK_NULL_HANDLE);
}

std::expected<SingleRenderPass, Error> SingleRenderPass::create(LogicalDevice& device, SingleRenderPass::Params params) noexcept
{
	//struct Attachment {
	//	enum class LoadOp : uint8_t {
	//		Load,
	//		Clear,
	//		DontCare
	//	};
	//	enum class StoreOp : uint8_t {
	//		Store,
	//		DontCare
	//	};
	//	LoadOp load;
	//	StoreOp store;
	//	RenderFormat format;
	//	Image::Samples samples;
	//	Image::Layout initialLayout;
	//	Image::Layout finalLayout;
	//};
	//struct Subpass {
	//	nyan::StaticVector<uint8_t, maxOutputAttachments> resolves;
	//	nyan::StaticVector<uint8_t, maxOutputAttachments> colors;
	//	enum class DepthMode : uint8_t {
	//		Attachment,
	//		Read,
	//	} depthStencil;
	//};
	nyan::StaticVector<VkAttachmentDescription, maxOutputAttachments + 1> attachments;
	nyan::StaticVector<VkAttachmentReference, maxOutputAttachments> colorAttachments;
	nyan::StaticVector<VkAttachmentReference, maxOutputAttachments> resolveAttachments;
	VkAttachmentReference depthStencilAttachment{
		.attachment {VK_ATTACHMENT_UNUSED},
		.layout {VK_IMAGE_LAYOUT_UNDEFINED }
	};
	uint32_t count{ 0 };
	for (const auto& attachment : params.attachments) {
		nyan::ignore = attachments.push_back(VkAttachmentDescription {
			.flags { 0 }, //VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT //VkAttachmentDescriptionFlags   
			.format {static_cast<VkFormat>(attachment.format)},
			.samples {static_cast<VkSampleCountFlagBits>(attachment.samples)},
			.loadOp {static_cast<VkAttachmentLoadOp>(attachment.load)},
			.storeOp {static_cast<VkAttachmentStoreOp>(attachment.store)},
			.stencilLoadOp {VK_ATTACHMENT_LOAD_OP_DONT_CARE},//{static_cast<VkAttachmentLoadOp>(attachment.load)},
			.stencilStoreOp {VK_ATTACHMENT_STORE_OP_DONT_CARE},//{static_cast<VkAttachmentStoreOp>(attachment.store)},
			.initialLayout {static_cast<VkImageLayout>(convert_layout(attachment.initialLayout))},
			.finalLayout {static_cast<VkImageLayout>(convert_layout(attachment.finalLayout))},
		});
		if (!is_depth_stencil(attachment.format)) {

			if (attachment.samples != Samples::C1)
				nyan::ignore = resolveAttachments.push_back(VkAttachmentReference{ .attachment {count++}, .layout{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} });
			else
				nyan::ignore = colorAttachments.push_back(VkAttachmentReference{ .attachment {count++}, .layout{VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL} });
		}
		else {
			::assert(depthStencilAttachment.attachment == VK_ATTACHMENT_UNUSED);
			if (depthStencilAttachment.attachment != VK_ATTACHMENT_UNUSED) [[unlikely]]
				return std::unexpected{VK_ERROR_UNKNOWN};
			depthStencilAttachment.attachment = count++;
			depthStencilAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
	}

	if (colorAttachments.size() != resolveAttachments.size() && !resolveAttachments.empty()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}

	VkSubpassDescription subpass
	{
		.flags  {0},											//VkSubpassDescriptionFlags       
		.pipelineBindPoint {VK_PIPELINE_BIND_POINT_GRAPHICS },	//VkPipelineBindPoint             
		.inputAttachmentCount {0},								//uint32_t                        
		.pInputAttachments {nullptr},		//param						//const VkAttachmentReference*    
		.colorAttachmentCount {static_cast<uint32_t>(colorAttachments.size())},								//uint32_t                        
		.pColorAttachments {colorAttachments.data()},		//param						//const VkAttachmentReference*    
		.pResolveAttachments {resolveAttachments.empty()? nullptr : resolveAttachments.data()},	//const VkAttachmentReference*    
		.pDepthStencilAttachment {&depthStencilAttachment},//param						//const VkAttachmentReference*    
		.preserveAttachmentCount {0},							//uint32_t                        
		.pPreserveAttachments {nullptr},	//have to deduce			//const uint32_t*                 
	};

	std::array<VkSubpassDependency, 2> dependencies{ 
		VkSubpassDependency {
			.srcSubpass {VK_SUBPASS_EXTERNAL},
			.dstSubpass {0},
			.srcStageMask {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			.dstStageMask {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			.srcAccessMask {VK_ACCESS_NONE_KHR},
			.dstAccessMask {VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
			.dependencyFlags {0},
		}, 
		VkSubpassDependency {
			.srcSubpass {0},
			.dstSubpass {VK_SUBPASS_EXTERNAL},
			.srcStageMask {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			.dstStageMask {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
			.srcAccessMask {VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT},
			.dstAccessMask {VK_ACCESS_NONE_KHR},
			.dependencyFlags {0},
		}
	};

	VkRenderPassCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO},
		.pNext {nullptr},
		.flags {0},
		.attachmentCount {static_cast<uint32_t>(attachments.size())},
		.pAttachments {attachments.data()},
		.subpassCount {1},
		.pSubpasses {&subpass},
		.dependencyCount {static_cast<uint32_t>(dependencies.size())},
		.pDependencies {dependencies.data()}
	};



	VkRenderPass handle{ VK_NULL_HANDLE };

	if (auto result = device.get_device().vkCreateRenderPass(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	Data data{
		.numColorAttachments {static_cast<uint32_t>(colorAttachments.size())}
	};

	for (const auto& attachment : params.attachments)
		nyan::ignore = data.clearValues.push_back(attachment.clearValue);

	return SingleRenderPass{ device.get_device(), device.get_deletion_queue(), handle, std::move(data) };
}

SingleRenderPass::SingleRenderPass(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkRenderPass handle, Data data) noexcept :
	RenderPass(device, deletionQueue, handle, std::move(data))
{

}