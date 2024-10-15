module;

#include <cassert>
#include <expected>
#include <utility>
#include <span>

#include "volk.h"

module NYANVulkan;
import NYANData;
import NYANLog;

using namespace nyan::vulkan;

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept :
	Object(*other.ptr_device, other.m_handle),
	r_deletionQueue(other.r_deletionQueue)
{
	other.m_handle = VK_NULL_HANDLE;
}

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept
{
	if (this != std::addressof(other))
	{
		std::swap(ptr_device, other.ptr_device);
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

PipelineLayout::~PipelineLayout() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_handle);
}

std::expected<PipelineLayout, Error> nyan::vulkan::PipelineLayout::create(const LogicalDeviceWrapper& deviceWrapper, DeletionQueue& deletionQueue, std::span<DescriptorSetLayout> descriptorSetLayouts) noexcept
{
	VkPushConstantRange range{
		.stageFlags = VK_SHADER_STAGE_ALL,
		.offset = 0,
		.size = pushConstantSize,
	};
	DynamicArray<VkDescriptorSetLayout> setLayouts;
	if (!setLayouts.reserve(descriptorSetLayouts.size())) [[unlikely]]
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };

	for (const auto& layout : descriptorSetLayouts)
		if (!setLayouts.push_back(layout.get_handle())) [[unlikely]]
			return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
		.pSetLayouts = setLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
	};

	VkPipelineLayout handle{ VK_NULL_HANDLE };
	if (auto result = deviceWrapper.vkCreatePipelineLayout(&pipelineLayoutCreateInfo, &handle); result != VK_SUCCESS) [[unlikely]] 
		return std::unexpected{ result };

	return PipelineLayout{deviceWrapper, handle, deletionQueue};
}

PipelineLayout::PipelineLayout(const LogicalDeviceWrapper& deviceWrapper, VkPipelineLayout pipelineLayout, DeletionQueue& deletionQueue) noexcept :
	Object(deviceWrapper, pipelineLayout),
	r_deletionQueue(deletionQueue)
{
	assert(m_handle != VK_NULL_HANDLE);
}