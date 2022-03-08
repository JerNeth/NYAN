#include "DescriptorSet.h"
#include "LogicalDevice.h"
vulkan::DescriptorSetAllocator::DescriptorSetAllocator(LogicalDevice& parent, const DescriptorSetLayout& layout) :
r_device(parent)
{
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (uint32_t i = 0; i < r_device.get_thread_count(); i++)
		perThread.emplace_back(new PerThread());


	for (uint32_t i = 0; i < layout.descriptors.size(); i++) {
		const auto& descriptorLayout = layout.descriptors[i];
		if (descriptorLayout.stages.any()) {
			uint32_t poolSize = descriptorLayout.arraySize * MAX_SETS_PER_POOL;
			//VkSampler immutableSampler = layout.immutableSampler.test(i) ? r_device.get_default_sampler(layout.immutableSamplers.get(i))->get_handle() : VK_NULL_HANDLE;
			VkSampler immutableSampler = VK_NULL_HANDLE;
			assert(descriptorLayout.arraySize != 0);
			bindings.push_back(VkDescriptorSetLayoutBinding
				{
					.binding = i,
					.descriptorType = static_cast<VkDescriptorType>(descriptorLayout.type),
					.descriptorCount = descriptorLayout.arraySize,
					.stageFlags = static_cast<VkShaderStageFlags>(descriptorLayout.stages.to_ulong()),
					.pImmutableSamplers = nullptr
				}
			);
			poolSizes.push_back({ static_cast<VkDescriptorType>(descriptorLayout.type), poolSize });
		}
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	if (auto result = vkCreateDescriptorSetLayout(r_device.m_device, &createInfo, r_device.m_allocator, &m_layout); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
}


vulkan::DescriptorSetAllocator::~DescriptorSetAllocator()
{
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(r_device.m_device, m_layout, r_device.m_allocator);
	clear();
}

std::pair<VkDescriptorSet, bool> vulkan::DescriptorSetAllocator::find(unsigned threadId, Utility::HashValue hash)
{
	auto& state = *perThread[threadId];
	if (state.removeStale) {
		for (auto& hashIt : state.stale)
			state.hashMap.remove(hashIt);
		state.stale.clear();
		state.removeStale = false;
	}
	if (auto r = state.hashMap.get(hash); r) {
		//TODO technically memory "leak" i.e. we never clear stale
		state.stale.push_back(hash);
		return {*r, true};
	}
	if (!state.vacant.empty()) {
		state.stale.push_back(hash);
		auto r = state.vacant.back();
		state.vacant.pop_back();
		state.hashMap.insert(hash, r);
		return { r, false };
	}
	VkDescriptorPool pool{ VK_NULL_HANDLE };
	VkDescriptorPoolCreateInfo createInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = MAX_SETS_PER_POOL,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.empty() ? nullptr: poolSizes.data()
	};
	if (auto result = vkCreateDescriptorPool(r_device.m_device, &createInfo, r_device.m_allocator, &pool); result != VK_SUCCESS) {
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorPool, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorPool, out of device memory");
		}
		if (result == VK_ERROR_FRAGMENTATION_EXT) {
			throw std::runtime_error("VK: could not create DescriptorPool, fragmentation error");
		}
		else {
			throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
		}
	}
	state.vacant.resize(MAX_SETS_PER_POOL);
	std::vector<VkDescriptorSetLayout> layouts(MAX_SETS_PER_POOL, m_layout);
	VkDescriptorSetAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.descriptorPool = pool,
		.descriptorSetCount = MAX_SETS_PER_POOL,
		.pSetLayouts = layouts.data()
	};
	if (auto result = vkAllocateDescriptorSets(r_device.m_device, &allocateInfo, state.vacant.data()); result != VK_SUCCESS) {
		throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
	}
	state.pools.push_back(pool);
	state.stale.push_back(hash);
	auto r = state.vacant.back();
	state.vacant.pop_back();
	state.hashMap.insert(hash, r);
	return { r, false };
}


VkDescriptorSetLayout vulkan::DescriptorSetAllocator::get_layout() const noexcept
{
	return m_layout;
}

void vulkan::DescriptorSetAllocator::clear()
{
	for (auto& state : perThread) {
		state->hashMap.clear();
		state->vacant.clear();
		state->stale.clear();
		for (auto& pool : state->pools) {
			//vkResetDescriptorPool(r_device.m_device, pool, 0);
			vkDestroyDescriptorPool(r_device.m_device, pool, r_device.m_allocator);
		}
		state->pools.clear();

	}
}
