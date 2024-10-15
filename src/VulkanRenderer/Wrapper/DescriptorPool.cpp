module;

//#include <array>
//#include <cassert>
//#include <expected>
//#include <utility>
//#include <vector>

#include "volk.h"

module NYANVulkan;
import std;

using namespace nyan::vulkan;

DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept :
	Object(*other.ptr_device, other.m_handle),
	r_deletionQueue(other.r_deletionQueue),
	m_sets(std::move(other.m_sets))
{
	other.m_handle = VK_NULL_HANDLE;
}

DescriptorPool& nyan::vulkan::DescriptorPool::operator=(
	DescriptorPool&& other) noexcept
{
	if(this != std::addressof(other))
	{
		::assert(ptr_device == other.ptr_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_sets, other.m_sets);
	}
	return *this;
}

DescriptorPool::~DescriptorPool() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_handle);
}

std::vector<DescriptorSet>& DescriptorPool::get_sets() noexcept
{
	return m_sets;
}

std::expected<DescriptorPool, Error> DescriptorPool::create(LogicalDevice& device, const DescriptorSetLayout& layout, uint32_t numDescriptorSets) noexcept
{
	const auto& info = layout.get_info();
	std::array poolSizes{
		VkDescriptorPoolSize {
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::storageBufferBinding),
			.descriptorCount = info.storageBufferCount
		},
		VkDescriptorPoolSize{
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::uniformBufferBinding),
			.descriptorCount = info.uniformBufferCount
		},
		VkDescriptorPoolSize{
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::samplerBinding),
			.descriptorCount = info.samplerCount
		},
		VkDescriptorPoolSize{
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::sampledImageBinding),
			.descriptorCount = info.sampledImageCount
		},
		VkDescriptorPoolSize{
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::storageImageBinding),
			.descriptorCount = info.storageImageCount
		},
		VkDescriptorPoolSize{
			.type = DescriptorSetLayout::bindless_binding_to_type(DescriptorSetLayout::accelerationStructureBinding),
			.descriptorCount = info.accelerationStructureCount
		}
	};

	if (!device.get_enabled_extensions().accelerationStructure)
		poolSizes[5].descriptorCount = 0;

	uint32_t bindingCount = 0;
	for (uint32_t i = 0; i < poolSizes.size(); ++i)
		if (poolSizes[i].descriptorCount)
			bindingCount = i + 1;

	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = numDescriptorSets,
		.poolSizeCount = bindingCount,
		.pPoolSizes = poolSizes.data(),
	};
	const auto& deviceWrapper = device.get_device();

	VkDescriptorPool pool{ VK_NULL_HANDLE };

	if (auto result = deviceWrapper.vkCreateDescriptorPool(&poolCreateInfo, &pool); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ Error{result} };

	std::vector setLayouts{ numDescriptorSets, layout.get_handle() };
	std::vector<VkDescriptorSet> setHandles{ numDescriptorSets, VK_NULL_HANDLE };

	VkDescriptorSetAllocateInfo allocateInfo {
		.sType {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO},
		.pNext {nullptr},
		.descriptorPool {pool},
		.descriptorSetCount {numDescriptorSets},
		.pSetLayouts {setLayouts.data()}
	};
	if (const auto result = deviceWrapper.vkAllocateDescriptorSets(&allocateInfo, setHandles.data()); result != VK_SUCCESS) [[unlikely]]
	{
		device.get_deletion_queue().queue_deletion(pool);
		return std::unexpected{ Error{result} };
	}
	std::vector<DescriptorSet> sets;
	for(auto setHandle : setHandles)
	{
		auto set = DescriptorSet::create(deviceWrapper, setHandle, layout);
		if(!set) [[unlikely]]
			return std::unexpected{ set.error() };
		sets.emplace_back(std::move(*set));
	}
	
	return DescriptorPool{deviceWrapper, pool, device.get_deletion_queue(), sets };
}

DescriptorPool::DescriptorPool(const LogicalDeviceWrapper& deviceWrapper, const VkDescriptorPool handle,
	DeletionQueue& deletionQueue, std::vector<DescriptorSet>& sets) noexcept :
	Object(deviceWrapper, handle),
	r_deletionQueue(deletionQueue)
{
	for (auto& set : sets)
		m_sets.emplace_back(std::move(set));
}

void DescriptorPool::reset() noexcept
{
	::assert(false);
	//Don't know why I would reset, hide for now
	m_sets.clear();
	//Synchronization necessary
	auto result = ptr_device->vkResetDescriptorPool(m_handle, 0);
	//Only valid result according to docs is Success, but idk
	::assert(result == VK_SUCCESS);
}
