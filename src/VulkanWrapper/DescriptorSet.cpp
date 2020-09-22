#include "DescriptorSet.h"
#include "LogicalDevice.h"
Vulkan::DescriptorSetAllocator::DescriptorSetAllocator(LogicalDevice& parent, const DescriptorSetLayout& layout) :
r_parent(parent)
{
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;
	for (int i = 0; i < 1; i++)
		perThread.emplace_back(new PerThread());


	for (uint32_t i = 0; i < MAX_BINDINGS; i++) {
		if (layout.stages[i]) {
			uint32_t poolSize = layout.arraySizes[i] * MAX_SETS_PER_POOL;
			if (layout.imageSampler.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
				{
					.binding = i,
					.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
					.descriptorCount = layout.arraySizes[i],
					.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
					.pImmutableSamplers = nullptr
				}
				);
				poolSizes.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, poolSize});
			}
			if (layout.sampledBuffer.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, poolSize });
			}
			if (layout.storageImage.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, poolSize });
			}
			if (layout.uniformBuffer.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				); 
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, poolSize });
			}
			if (layout.storageBuffer.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, poolSize });
			}
			if (layout.inputAttachment.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, poolSize });
			}
			if (layout.separateImage.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, poolSize });
			}
			if (layout.seperateSampler.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				); 
				poolSizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, poolSize });
			}
		}
	}

	VkDescriptorSetLayoutCreateInfo createInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};
	if (auto result = vkCreateDescriptorSetLayout(r_parent.m_device, &createInfo, r_parent.m_allocator, &m_layout); result != VK_SUCCESS) {
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


Vulkan::DescriptorSetAllocator::~DescriptorSetAllocator()
{
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(r_parent.m_device, m_layout, r_parent.m_allocator);
	clear();
}

std::pair<VkDescriptorSet, bool> Vulkan::DescriptorSetAllocator::find(unsigned threadId, Utility::HashValue hash)
{
	auto& state = *perThread[threadId];
	if (state.removeStale) {
		for (auto& hash : state.stale)
			state.hashMap.remove(hash);
		state.stale.clear();
		state.removeStale = false;
	}
	if (auto r = state.hashMap.get(hash); r) {
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
	VkDescriptorPool pool;
	VkDescriptorPoolCreateInfo createInfo {
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.maxSets = MAX_SETS_PER_POOL,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.empty() ? nullptr: poolSizes.data()
	};
	if (auto result = vkCreateDescriptorPool(r_parent.m_device, &createInfo, r_parent.m_allocator, &pool); result != VK_SUCCESS) {
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
	if (auto result = vkAllocateDescriptorSets(r_parent.m_device, &allocateInfo, state.vacant.data()); result != VK_SUCCESS) {
		throw std::runtime_error("VK: error " + std::to_string((int)result) + std::string(" in ") + std::string(__PRETTY_FUNCTION__) + std::to_string(__LINE__));
	}

	state.stale.push_back(hash);
	auto r = state.vacant.back();
	state.vacant.pop_back();
	state.hashMap.insert(hash, r);
	return { r, false };
}


VkDescriptorSetLayout Vulkan::DescriptorSetAllocator::get_layout() const noexcept
{
	return m_layout;
}

void Vulkan::DescriptorSetAllocator::clear()
{
	for (auto& state : perThread) {
		state->hashMap.clear();
		state->vacant.clear();
		state->stale.clear();
		for (auto& pool : state->pools) {
			vkResetDescriptorPool(r_parent.m_device, pool, 0);
			vkDestroyDescriptorPool(r_parent.m_device, pool, r_parent.m_allocator);
		}
		state->pools.clear();
	}
}
