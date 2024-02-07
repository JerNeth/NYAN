module;

#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;
import NYANData;
import NYANLog;

using namespace nyan::vulkan::wrapper;

PipelineLayout::PipelineLayout(PipelineLayout&& other) noexcept :
	Object(other.r_device, other.m_handle),
	r_deletionQueue(other.r_deletionQueue)
{
	other.m_handle = VK_NULL_HANDLE;
}

PipelineLayout& PipelineLayout::operator=(PipelineLayout&& other) noexcept
{
	if (this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

PipelineLayout::~PipelineLayout() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_pipeline_layout_deletion(m_handle);
}

std::expected<PipelineLayout, Error> nyan::vulkan::wrapper::PipelineLayout::create(const LogicalDeviceWrapper& deviceWrapper, DeletionQueue& deletionQueue, std::span<DescriptorSetLayout> descriptorSetLayouts) noexcept
{
	VkPushConstantRange range{
		.stageFlags = VK_SHADER_STAGE_ALL,
		.offset = 0,
		.size = pushConstantSize,
	};
	util::data::DynArray<VkDescriptorSetLayout> setLayouts;
	if (!setLayouts.reserve(descriptorSetLayouts.size()))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };

	for (const auto& layout : descriptorSetLayouts)
		if (!setLayouts.push_back(layout.get_handle()))
			return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };

	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		.setLayoutCount = static_cast<uint32_t>(setLayouts.size()),
		.pSetLayouts = setLayouts.data(),
		.pushConstantRangeCount = 1,
		.pPushConstantRanges = &range
	};

	VkPipelineLayout handle{ VK_NULL_HANDLE };
	if (auto result = deviceWrapper.vkCreatePipelineLayout(&pipelineLayoutCreateInfo, &handle); result != VK_SUCCESS) {
		return std::unexpected{ result };
	}
	return PipelineLayout{deviceWrapper, handle, deletionQueue};
}

PipelineLayout::PipelineLayout(const LogicalDeviceWrapper& deviceWrapper, VkPipelineLayout pipelineLayout, DeletionQueue& deletionQueue) noexcept :
	Object(deviceWrapper, pipelineLayout),
	r_deletionQueue(deletionQueue)
{
	assert(m_handle != VK_NULL_HANDLE);
}