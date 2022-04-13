#include "DescriptorSet.h"
#include "LogicalDevice.h"
#include <numeric>
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
			//VkSampler immutableSampler = VK_NULL_HANDLE;
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
		.pNext = nullptr,
		.flags = 0,
		.maxSets = MAX_SETS_PER_POOL,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.empty() ? nullptr: poolSizes.data(),
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
vulkan::DescriptorSet::DescriptorSet(DescriptorPool& pool, VkDescriptorSet vkHandle) :
	r_pool(pool),
	m_vkHandle(vkHandle)
{
}

uint32_t vulkan::DescriptorSet::set_storage_buffer(const VkDescriptorBufferInfo& bufferInfo)
{
	uint32_t arrayElement;
	if (!m_storageBufferFsi.empty()) {
		arrayElement = m_storageBufferFsi.back();
		m_storageBufferFsi.pop_back();
	}
	else {
		arrayElement = m_storageBufferCount++;
	}
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageBufferBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_uniform_buffer(const VkDescriptorBufferInfo& bufferInfo)
{
	uint32_t arrayElement;
	if (!m_uniformBufferFsi.empty()) {
		arrayElement = m_uniformBufferFsi.back();
		m_uniformBufferFsi.pop_back();
	}
	else {
		arrayElement = m_uniformBufferCount++;
	}
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = uniformBufferBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(uniformBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampler(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement;
	if (!m_samplerFsi.empty()) {
		arrayElement = m_samplerFsi.back();
		m_samplerFsi.pop_back();
	}
	else {
		arrayElement = m_samplerCount++;
	}
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = samplerBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(samplerBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampled_image(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement;
	if (!m_sampledImageFsi.empty()) {
		arrayElement = m_sampledImageFsi.back();
		m_sampledImageFsi.pop_back();
	}
	else {
		arrayElement = m_sampledImageCount++;
	}
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = sampledImageBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(sampledImageBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_storage_image(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement;
	if (!m_storageImageFsi.empty()) {
		arrayElement = m_storageImageFsi.back();
		m_storageImageFsi.pop_back();
	}
	else {
		arrayElement = m_storageImageCount++;
	}
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageImageBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageImageBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_acceleration_structure(const VkAccelerationStructureKHR& accelerationStructure)
{
	uint32_t arrayElement;
	if (!m_accelerationStructureFsi.empty()) {
		arrayElement = m_accelerationStructureFsi.back();
		m_accelerationStructureFsi.pop_back();
	}
	else {
		arrayElement = m_accelerationStructureCount++;
	}
	VkWriteDescriptorSetAccelerationStructureKHR accWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
		.accelerationStructureCount = 1,
		.pAccelerationStructures = &accelerationStructure
	};
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = &accWrite,
		.dstSet = m_vkHandle,
		.dstBinding = accelerationStructureBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(accelerationStructureBinding)
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
	return arrayElement;
}
void vulkan::DescriptorSet::set_storage_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo)
{
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageBufferBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_uniform_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo)
{
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = uniformBufferBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(uniformBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_sampler(uint32_t idx, const VkDescriptorImageInfo& imageInfo)
{
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = samplerBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(samplerBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_sampled_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo)
{
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = sampledImageBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(sampledImageBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_storage_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo)
{
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageImageBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageImageBinding),
		.pImageInfo = &imageInfo,
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_acceleration_structure(uint32_t idx, const VkAccelerationStructureKHR& accelerationStructure)
{
	VkWriteDescriptorSetAccelerationStructureKHR accWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
		.accelerationStructureCount = 1,
		.pAccelerationStructures = &accelerationStructure
	};
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = &accWrite,
		.dstSet = m_vkHandle,
		.dstBinding = accelerationStructureBinding,
		.dstArrayElement = idx,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(accelerationStructureBinding)
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);
}
std::vector<uint32_t> vulkan::DescriptorSet::set_storage_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos)
{
	std::vector<uint32_t> arrayElements(bufferInfos.size());

	std::iota(arrayElements.begin(), arrayElements.end(), m_storageBufferCount);
	m_storageBufferCount += static_cast<uint32_t>(bufferInfos.size());

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageBufferBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(bufferInfos.size()),
		.descriptorType = bindless_binding_to_type(storageBufferBinding),
		.pBufferInfo = bufferInfos.data()
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_uniform_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos) 
{
	std::vector<uint32_t> arrayElements(bufferInfos.size());

	std::iota(arrayElements.begin(), arrayElements.end(), m_uniformBufferCount);
	m_uniformBufferCount += static_cast<uint32_t>(bufferInfos.size());

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = uniformBufferBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(bufferInfos.size()),
		.descriptorType = bindless_binding_to_type(uniformBufferBinding),
		.pBufferInfo = bufferInfos.data()
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_sampler(const std::vector<VkDescriptorImageInfo>& imageInfos) 
{
	std::vector<uint32_t> arrayElements(imageInfos.size());

	std::iota(arrayElements.begin(), arrayElements.end(), m_samplerCount);
	m_samplerCount += static_cast<uint32_t>(imageInfos.size());

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = samplerBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
		.descriptorType = bindless_binding_to_type(samplerBinding),
		.pImageInfo = imageInfos.data()
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_sampled_image(const std::vector<VkDescriptorImageInfo>& imageInfos) 
{
	std::vector<uint32_t> arrayElements(imageInfos.size());

	std::iota(arrayElements.begin(), arrayElements.end(), m_sampledImageCount);
	m_sampledImageCount += static_cast<uint32_t>(imageInfos.size());

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = sampledImageBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
		.descriptorType = bindless_binding_to_type(sampledImageBinding),
		.pImageInfo = imageInfos.data()
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_storage_image(const std::vector<VkDescriptorImageInfo>& imageInfos) 
{
	std::vector<uint32_t> arrayElements(imageInfos.size());

	std::iota(arrayElements.begin(), arrayElements.end(), m_storageImageCount);
	m_storageImageCount += static_cast<uint32_t>(imageInfos.size());

	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageImageBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(imageInfos.size()),
		.descriptorType = bindless_binding_to_type(storageImageBinding),
		.pImageInfo = imageInfos.data()
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_acceleration_structure(const std::vector<VkAccelerationStructureKHR>& accelerationStructures) 
{
	std::vector<uint32_t> arrayElements(accelerationStructures.size());
	//if (!m_storageBufferFsi.empty()) {
	//	arrayElement = m_storageBufferFsi.back();
	//	m_storageBufferFsi.pop_back();
	//}
	//else {
	std::iota(arrayElements.begin(), arrayElements.end(), m_accelerationStructureCount);
	m_accelerationStructureCount += static_cast<uint32_t>(accelerationStructures.size());
	//}
	VkWriteDescriptorSetAccelerationStructureKHR accWrite{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
		.accelerationStructureCount = static_cast<uint32_t>(accelerationStructures.size()),
		.pAccelerationStructures = accelerationStructures.data()
	};
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.pNext = &accWrite,
		.dstSet = m_vkHandle,
		.dstBinding = accelerationStructureBinding,
		.dstArrayElement = arrayElements.front(),
		.descriptorCount = static_cast<uint32_t>(accelerationStructures.size()),
		.descriptorType = bindless_binding_to_type(accelerationStructureBinding)
	};
	vkUpdateDescriptorSets(r_pool.r_device.get_device(), 1, &write, 0, nullptr);

	return arrayElements;
}


vulkan::DescriptorSet::operator VkDescriptorSet() const noexcept
{
	return m_vkHandle;
}
VkDescriptorSet vulkan::DescriptorSet::get_set() const noexcept
{
	return m_vkHandle;
}

VkDescriptorType vulkan::DescriptorSet::bindless_binding_to_type(uint32_t binding) noexcept
{
	switch (binding) {
	case storageBufferBinding:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case uniformBufferBinding:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case samplerBinding:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case sampledImageBinding:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case storageImageBinding:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case accelerationStructureBinding:
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	default:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	}
}

vulkan::DescriptorPool::DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo) :
	r_device(device),
	m_createInfo(createInfo)
{
	const auto& vulkan12Properties = r_device.get_physical_device().get_vulkan12_properties();
	const auto& rtProperties = r_device.get_physical_device().get_acceleration_structure_properties();

	if (m_createInfo.storage_buffer_count > vulkan12Properties.maxDescriptorSetUpdateAfterBindStorageBuffers) {
		m_createInfo.storage_buffer_count = vulkan12Properties.maxDescriptorSetUpdateAfterBindStorageBuffers;
		Utility::log("Not enough bindless storage buffers");
	}	
	if (m_createInfo.uniform_buffer_count > vulkan12Properties.maxDescriptorSetUpdateAfterBindUniformBuffers) {
		m_createInfo.uniform_buffer_count = vulkan12Properties.maxDescriptorSetUpdateAfterBindUniformBuffers;
		Utility::log("Not enough bindless uniform buffers");
	}
	if (m_createInfo.sampled_image_count > vulkan12Properties.maxDescriptorSetUpdateAfterBindSampledImages) {
		m_createInfo.sampled_image_count = vulkan12Properties.maxDescriptorSetUpdateAfterBindSampledImages;
		Utility::log("Not enough bindless sampled images");
	}
	if (m_createInfo.storage_image_count > vulkan12Properties.maxDescriptorSetUpdateAfterBindStorageImages) {
		m_createInfo.storage_image_count = vulkan12Properties.maxDescriptorSetUpdateAfterBindStorageImages;
		Utility::log("Not enough bindless storage images");
	}
	if (m_createInfo.sampler_count > vulkan12Properties.maxDescriptorSetUpdateAfterBindSamplers) {
		m_createInfo.sampler_count = vulkan12Properties.maxDescriptorSetUpdateAfterBindSamplers;
		Utility::log("Not enough bindless samplers");
	}
	if (m_createInfo.acceleration_structure_count > rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
		m_createInfo.acceleration_structure_count = rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures;
		Utility::log("Not enough bindless acceleration structures");
	}


	std::array bindings{
		VkDescriptorSetLayoutBinding
		{
			.binding = storageBufferBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(storageBufferBinding),
			.descriptorCount = m_createInfo.storage_buffer_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = uniformBufferBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(uniformBufferBinding),
			.descriptorCount = m_createInfo.uniform_buffer_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = samplerBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(samplerBinding),
			.descriptorCount = m_createInfo.sampler_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = sampledImageBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(sampledImageBinding),
			.descriptorCount = m_createInfo.sampled_image_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = storageImageBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(storageImageBinding),
			.descriptorCount = m_createInfo.storage_image_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = accelerationStructureBinding,
			.descriptorType = DescriptorSet::bindless_binding_to_type(accelerationStructureBinding),
			.descriptorCount = m_createInfo.acceleration_structure_count,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
	};
	std::array poolSizes{
		VkDescriptorPoolSize {
			.type = bindings[0].descriptorType,
			.descriptorCount = bindings[0].descriptorCount
		},
		VkDescriptorPoolSize {
			.type = bindings[1].descriptorType,
			.descriptorCount = bindings[1].descriptorCount
		},
		VkDescriptorPoolSize {
			.type = bindings[2].descriptorType,
			.descriptorCount = bindings[2].descriptorCount
		},
		VkDescriptorPoolSize {
			.type = bindings[3].descriptorType,
			.descriptorCount = bindings[3].descriptorCount
		},
		VkDescriptorPoolSize {
			.type = bindings[4].descriptorType,
			.descriptorCount = bindings[4].descriptorCount
		},
		VkDescriptorPoolSize {
			.type = bindings[5].descriptorType,
			.descriptorCount = bindings[5].descriptorCount
		},
	};
	std::array<VkDescriptorBindingFlags, bindings.size()> flags{};

	flags.fill(VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT |
		//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT |
		VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT);

	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindingFlags = flags.data()
	};
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &flagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = static_cast<uint32_t>(bindings.size()),
		.pBindings = bindings.data()
	};

	if (auto result = vkCreateDescriptorSetLayout(r_device.get_device(), &setLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
		Utility::log_error(std::to_string((int)result));
		if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of host memory");
		}
		if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
			throw std::runtime_error("VK: could not create DescriptorSetLayout, out of device memory");
		}
		else {
			throw std::runtime_error("VK: error");
		}
	}

	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};
	if (auto result = vkCreateDescriptorPool(r_device.get_device(), &poolCreateInfo, r_device.get_allocator(), &m_pool); result != VK_SUCCESS) {
		Utility::log_error(std::to_string((int)result));
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
			throw std::runtime_error("VK: error");
		}
	}

}

vulkan::DescriptorPool::~DescriptorPool() noexcept
{
	if (m_pool != VK_NULL_HANDLE)
		vkDestroyDescriptorPool(r_device.get_device(), m_pool, r_device.get_allocator());
	if (m_layout != VK_NULL_HANDLE)
		vkDestroyDescriptorSetLayout(r_device.get_device(), m_layout, r_device.get_allocator());
}

vulkan::DescriptorSet vulkan::DescriptorPool::allocate_set()
{
	VkDescriptorSet set{ VK_NULL_HANDLE };
	//VkDescriptorSetVariableDescriptorCountAllocateInfo variableAllocateInfo{
	//	.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO,
	//	.pNext = nullptr,
	//	.descriptorSetCount = 1,
	//	.pDescriptorCounts = nullptr,

	//};
	VkDescriptorSetAllocateInfo allocateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		.pNext = nullptr,
		.descriptorPool = m_pool,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_layout
	};
	if (auto result = vkAllocateDescriptorSets(r_device.get_device(), &allocateInfo, &set); result != VK_SUCCESS) {
		Utility::log_error(std::to_string((int)result));
		throw std::runtime_error("VK: error");
	}
	return { *this, set };
}
const vulkan::DescriptorCreateInfo& vulkan::DescriptorPool::get_info() const
{
	return m_createInfo;
}

VkDescriptorSetLayout vulkan::DescriptorPool::get_layout() const
{
	return m_layout;
}
