module;

#include <cassert>
#include <expected>
#include <utility>
#include <source_location>

#include "magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

module NYANVulkan;
import NYANData;
import NYANLog;
import :LogicalDevice;
import :PhysicalDevice;

using namespace nyan::vulkan;
using namespace nyan;

Framebuffer::Framebuffer(Framebuffer&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	m_data(other.m_data),
	r_deletionQueue(other.r_deletionQueue)
{
}

Framebuffer& Framebuffer::operator=(Framebuffer&& other) noexcept
{
	assert(ptr_device == other.ptr_device);
	assert(std::addressof(r_deletionQueue) == std::addressof(other.r_deletionQueue));
	if (this != std::addressof(other)) {
		std::swap(m_handle, other.m_handle);
		std::swap(m_data, other.m_data);
	}
	return *this;
}

Framebuffer::~Framebuffer() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_handle);
}

std::expected<Framebuffer, Error> Framebuffer::create(LogicalDevice& device, const Params& params) noexcept
{
	//A pointer to a standard-layout class may be converted (with reinterpret_cast) to a pointer to its first non-static data member and vice versa.
	static_assert(sizeof(ImageView) == sizeof(VkImageView));
	static_assert(std::is_standard_layout_v<ImageView> == true); 

	VkFramebufferCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO},
		.pNext {nullptr},
		.flags {0 },
		.renderPass {params.renderPass.get_handle()},
		.attachmentCount {static_cast<uint32_t>(params.attachments.size())},
		.pAttachments {reinterpret_cast<const VkImageView*>(params.attachments.data())},
		.width {params.width},
		.height {params.height},
		.layers {1}
	};

	if (createInfo.height > device.get_physical_device().get_properties().limits.maxFramebufferHeight ||
		createInfo.height == 0) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (createInfo.width > device.get_physical_device().get_properties().limits.maxFramebufferWidth ||
		createInfo.width == 0) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	VkFramebuffer handle{ VK_NULL_HANDLE };
	if (auto result = device.get_device().vkCreateFramebuffer(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]] 
		return std::unexpected{ VK_ERROR_UNKNOWN };

	return Framebuffer{ device.get_device(), device.get_deletion_queue(), handle, Data {.width {params.width}, .height {params.height}} };
}

uint16_t nyan::vulkan::Framebuffer::get_width() const noexcept
{
	return m_data.width;
}

uint16_t nyan::vulkan::Framebuffer::get_height() const noexcept
{
	return m_data.height;
}

Framebuffer::Framebuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkFramebuffer handle, Data data) noexcept :
	Object(device, handle),
	m_data(data),
	r_deletionQueue(deletionQueue)
{
	assert(m_handle != VK_NULL_HANDLE);
}

std::expected<ImagelessFramebuffer, Error> ImagelessFramebuffer::create(LogicalDevice& device) noexcept
{
	assert(device.get_physical_device().get_vulkan12_features().imagelessFramebuffer);
	if(!device.get_physical_device().get_vulkan12_features().imagelessFramebuffer) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	nyan::StaticVector< VkFramebufferAttachmentImageInfo, 8> attachmentImageInfos;

	VkFramebufferAttachmentsCreateInfo attachmentsCreateInfo{
		.sType {VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO},
		.pNext {nullptr},
		.attachmentImageInfoCount {static_cast<uint32_t>(attachmentImageInfos.size())},
		.pAttachmentImageInfos {attachmentImageInfos.data()}
	};

	VkFramebufferCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO},
		.pNext {&attachmentsCreateInfo},
		.flags {VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT },
		.renderPass {VK_NULL_HANDLE},
		.attachmentCount {0},
		.width {0},
		.height {0},
		.layers {1}
	};

	if (createInfo.height > device.get_physical_device().get_properties().limits.maxFramebufferHeight ||
		createInfo.height == 0) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (createInfo.width > device.get_physical_device().get_properties().limits.maxFramebufferWidth ||
		createInfo.width == 0) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	VkFramebuffer handle{VK_NULL_HANDLE};
	if (auto result = device.get_device().vkCreateFramebuffer(&createInfo, &handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ VK_ERROR_UNKNOWN };

	return ImagelessFramebuffer{device.get_device(), device.get_deletion_queue(), handle};
}

ImagelessFramebuffer::ImagelessFramebuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkFramebuffer handle) noexcept :
	Framebuffer(device, deletionQueue, handle, {})
{
}
