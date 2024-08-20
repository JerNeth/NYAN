module;

#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;
import :DescriptorSetLayout;

using namespace nyan::vulkan::wrapper;

DescriptorSet::DescriptorSet(DescriptorSet&& other) noexcept :
	Object(other.r_device, other.m_handle),
	r_layout(other.r_layout),
	m_bitmaps(std::move(other.m_bitmaps)),
	m_counts(std::move(other.m_counts))
{
	other.m_handle = VK_NULL_HANDLE;
}

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& other) noexcept
{
	assert(std::addressof(r_device) == std::addressof(other.r_device));
	assert(std::addressof(r_layout) == std::addressof(other.r_layout));
	if(this != std::addressof(other))
	{
		std::swap(m_handle, other.m_handle);
		std::swap(m_bitmaps, other.m_bitmaps);
		std::swap(m_counts, other.m_counts);
	}
	return *this;
}

template<typename T1, typename T2>
bool add_descriptor(T1& count, T2& bitmap, uint32_t& descriptor) noexcept
{

	auto bitmapIdx = bitmap.find_empty();
	if (!bitmapIdx)
		return false;
	descriptor = *bitmapIdx;

	bitmap.set(descriptor);
	count++;
	return true;

}

std::expected<StorageBufferDescriptor, Error> DescriptorSet::add(const StorageBuffer& buffer, const VkDeviceSize offset, const VkDeviceSize range) noexcept
{
	uint32_t descriptorValue{ 0 };

	if (!add_descriptor(m_counts.storageBuffers, m_bitmaps.storageBuffers, descriptorValue))
		return std::unexpected{VK_ERROR_UNKNOWN};

	StorageBufferDescriptor descriptor { descriptorValue };

	update(descriptor, buffer, offset, range);

	return descriptor;
}

void DescriptorSet::update(StorageBufferDescriptor descriptor, const StorageBuffer& buffer, VkDeviceSize offset, VkDeviceSize range) noexcept
{
	VkDescriptorBufferInfo bufferInfo{
	.buffer {buffer.get_handle()},
	.offset {offset},
	.range {range}
	};
	assert(bufferInfo.buffer != VK_NULL_HANDLE);
	assert(bufferInfo.range <= std::numeric_limits<uint32_t>::max()); //TODO: Query device, this is a practical common limit

	VkWriteDescriptorSet write{
		.sType {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET},
		.pNext {nullptr},
		.dstSet {m_handle},
		.dstBinding {DescriptorSetLayout::storageBufferBinding},
		.dstArrayElement {descriptor.value},
		.descriptorCount {1},
		.descriptorType {DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::storageBufferBinding)},
		.pImageInfo {nullptr},
		.pBufferInfo {&bufferInfo},
		.pTexelBufferView {nullptr}
	};

	r_device.vkUpdateDescriptorSets(1, &write, 0, nullptr);
}

template<typename T1, typename T2>
void remove_descriptor(T1& count, T2& bitmap, uint32_t descriptor) noexcept 
{
	assert(descriptor < bitmap.capacity());
	count -= bitmap.test(descriptor);
	bitmap.reset(descriptor);
}

void DescriptorSet::remove(StorageBufferDescriptor descriptor) noexcept
{
	remove_descriptor(m_counts.storageBuffers, m_bitmaps.storageBuffers, descriptor.value);
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