module;

#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;
import :DescriptorSetLayout;

using namespace nyan::vulkan::wrapper;

DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept :
	Object(other.r_device, other.m_handle),
	r_layout(other.r_layout)
{
	other.m_handle = VK_NULL_HANDLE;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
{
	if(this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		assert(std::addressof(r_layout) == std::addressof(other.r_layout));
		std::swap(m_handle, other.m_handle);
	}
	return *this;
}

std::expected<DescriptorSet, Error> DescriptorSet::create(const LogicalDeviceWrapper& deviceWrapper, const VkDescriptorSet handle, const DescriptorSetLayout& layout) noexcept
{
	Bitmaps bitmaps;
	if (!bitmaps.storageBuffers.reserve(layout.get_info().storageBufferCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };
	if (!bitmaps.uniformBuffers.reserve(layout.get_info().uniformBufferCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };
	if (!bitmaps.samplers.reserve(layout.get_info().samplerCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };
	if (!bitmaps.sampledImages.reserve(layout.get_info().sampledImageCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };
	if (!bitmaps.storageImages.reserve(layout.get_info().storageImageCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };
	if (!bitmaps.accelerationStructures.reserve(layout.get_info().accelerationStructureCount))
		return std::unexpected{ VK_ERROR_OUT_OF_HOST_MEMORY };

	return DescriptorSet{deviceWrapper, handle, layout, std::move(bitmaps)};
}

DescriptorSet::DescriptorSet(const LogicalDeviceWrapper& deviceWrapper, const VkDescriptorSet handle, const DescriptorSetLayout& layout, Bitmaps bitmaps) noexcept :
	Object(deviceWrapper, handle),
	r_layout(layout),
	m_bitmaps(std::move(bitmaps))
{
}