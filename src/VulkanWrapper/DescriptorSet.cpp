#include "DescriptorSet.h"
#include "LogicalDevice.h"
#include "Instance.h"
#include "Utility/Exceptions.h"
#include "descriptors.h"
#include "VulkanWrapper/PhysicalDevice.hpp"

namespace vulkan {
	static constexpr uint32_t storageBufferBinding = STORAGE_BUFFER_BINDING;
	static constexpr uint32_t uniformBufferBinding = UNIFORM_BUFFER_BINDING;
	static constexpr uint32_t samplerBinding = SAMPLER_BINDING;
	static constexpr uint32_t sampledImageBinding = SAMPLED_IMAGE_BINDING;
	static constexpr uint32_t storageImageBinding = STORAGE_IMAGE_BINDING;
	static constexpr uint32_t accelerationStructureBinding = ACCELERATION_STRUCTURE_BINDING;
};

vulkan::DescriptorSet::DescriptorSet(DescriptorPool& pool, VkDescriptorSet vkHandle) :
	r_pool(pool),
	m_vkHandle(vkHandle)
{
}

uint32_t vulkan::DescriptorSet::set_storage_buffer(const VkDescriptorBufferInfo& bufferInfo)
{
	uint32_t arrayElement = reserve_storage_buffer();
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageBufferBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_uniform_buffer(const VkDescriptorBufferInfo& bufferInfo)
{
	uint32_t arrayElement = reserve_uniform_buffer();
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = uniformBufferBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(uniformBufferBinding),
		.pBufferInfo = &bufferInfo,
	};
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampler(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement = reserve_sampler();
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = samplerBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(samplerBinding),
		.pImageInfo = &imageInfo,
	};
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampled_image(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement = reserve_sampled_image();
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = sampledImageBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(sampledImageBinding),
		.pImageInfo = &imageInfo,
	};
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_storage_image(const VkDescriptorImageInfo& imageInfo)
{
	uint32_t arrayElement = reserve_storage_image();
	VkWriteDescriptorSet write{
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_vkHandle,
		.dstBinding = storageImageBinding,
		.dstArrayElement = arrayElement,
		.descriptorCount = 1,
		.descriptorType = bindless_binding_to_type(storageImageBinding),
		.pImageInfo = &imageInfo,
	};
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_acceleration_structure(const VkAccelerationStructureKHR& accelerationStructure)
{
	uint32_t arrayElement = reserve_acceleration_structure();
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_storage_buffer()
{
	uint32_t arrayElement;
	if (!m_storageBufferFsi.empty()) {
		arrayElement = m_storageBufferFsi.back();
		m_storageBufferFsi.pop_back();
	}
	else {
		arrayElement = m_storageBufferCount++;
	}
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_uniform_buffer()
{
	uint32_t arrayElement;
	if (!m_uniformBufferFsi.empty()) {
		arrayElement = m_uniformBufferFsi.back();
		m_uniformBufferFsi.pop_back();
	}
	else {
		arrayElement = m_uniformBufferCount++;
	}
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_sampler()
{
	uint32_t arrayElement;
	if (!m_samplerFsi.empty()) {
		arrayElement = m_samplerFsi.back();
		m_samplerFsi.pop_back();
	}
	else {
		arrayElement = m_samplerCount++;
	}
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_sampled_image()
{
	uint32_t arrayElement;
	if (!m_sampledImageFsi.empty()) {
		arrayElement = m_sampledImageFsi.back();
		m_sampledImageFsi.pop_back();
	}
	else {
		arrayElement = m_sampledImageCount++;
	}
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_storage_image()
{
	uint32_t arrayElement;
	if (!m_storageImageFsi.empty()) {
		arrayElement = m_storageImageFsi.back();
		m_storageImageFsi.pop_back();
	}
	else {
		arrayElement = m_storageImageCount++;
	}
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_acceleration_structure()
{
	uint32_t arrayElement;
	if (!m_accelerationStructureFsi.empty()) {
		arrayElement = m_accelerationStructureFsi.back();
		m_accelerationStructureFsi.pop_back();
	}
	else {
		arrayElement = m_accelerationStructureCount++;
	}
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);

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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);

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
	r_pool.r_device.get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);

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
	r_pool.r_device.get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

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
	r_pool.r_device.get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);

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

constexpr VkDescriptorType vulkan::DescriptorSet::bindless_binding_to_type(uint32_t binding) noexcept
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

vulkan::DescriptorPool::DescriptorPool(LogicalDevice& device) :
	VulkanObject(device),
	m_createInfo(DescriptorCreateInfo{ 
		.storage_buffer_count {SSBO_COUNT},
		.uniform_buffer_count {UBO_COUNT},
		.sampler_count {SAMPLER_COUNT},
		.sampled_image_count {TEXTURE_COUNT},
		.storage_image_count {IMAGE_COUNT},
		.acceleration_structure_count {ACC_COUNT},
	})
{
	const auto& vulkan12Properties = r_device.get_physical_device().get_vulkan12_properties();
	const auto& rtProperties = r_device.get_physical_device().get_acceleration_structure_properties();

	if (m_createInfo.storage_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
		Utility::log().location().format("Not enough bindless storage buffers {} | {}", m_createInfo.storage_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
		m_createInfo.storage_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
	}
	if (m_createInfo.uniform_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
		Utility::log().location().format("Not enough bindless uniform buffers {} | {}", m_createInfo.uniform_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
		m_createInfo.uniform_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
	}
	if (m_createInfo.sampled_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
		Utility::log().location().format("Not enough bindless sampled images {} | {}", m_createInfo.sampled_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages);
		m_createInfo.sampled_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages;
	}
	if (m_createInfo.storage_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages) {
		Utility::log().location().format("Not enough bindless storage images {} | {}", m_createInfo.storage_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages);
		m_createInfo.storage_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages;
	}
	if (m_createInfo.sampler_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
		Utility::log().location().format("Not enough bindless samplers {} | {}", m_createInfo.sampler_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers);
		m_createInfo.sampler_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers;
	}
	if (m_createInfo.acceleration_structure_count > rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
		Utility::log().location().format("Not enough bindless acceleration structures {} | {}", m_createInfo.acceleration_structure_count, rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures);
		m_createInfo.acceleration_structure_count = rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures;
	}


	std::vector bindings{
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
	};


	std::vector poolSizes{
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
		}
	};
	if (m_createInfo.acceleration_structure_count) {
		bindings.push_back(VkDescriptorSetLayoutBinding
			{
				.binding = accelerationStructureBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(accelerationStructureBinding),
				.descriptorCount = m_createInfo.acceleration_structure_count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			});
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = bindings[5].descriptorType,
			.descriptorCount = bindings[5].descriptorCount
			});
	}
	std::vector<VkDescriptorBindingFlags> flags;

	VkDescriptorBindingFlags flag{ 0 };
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingPartiallyBound) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingUpdateUnusedWhilePending) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
	}
	flags.resize(bindings.size(), flag);
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingStorageBufferUpdateAfterBind) {
		flags[storageBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingUniformBufferUpdateAfterBind) {
		flags[uniformBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingSampledImageUpdateAfterBind) {
		flags[samplerBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		flags[sampledImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingStorageImageUpdateAfterBind) {
		flags[storageImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}

	if (m_createInfo.acceleration_structure_count) {
		if (r_device.get_physical_device().get_acceleration_structure_features().descriptorBindingAccelerationStructureUpdateAfterBind) {
			flags[accelerationStructureBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
	}

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

	if (auto result = r_device.get_device().vkCreateDescriptorSetLayout( &setLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}

	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};
	if (auto result = r_device.get_device().vkCreateDescriptorPool( &poolCreateInfo, r_device.get_allocator(), &m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}

}

vulkan::DescriptorPool::DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo) :
	VulkanObject(device),
	m_createInfo(createInfo)
{
	const auto& vulkan12Properties = r_device.get_physical_device().get_vulkan12_properties();
	const auto& rtProperties = r_device.get_physical_device().get_acceleration_structure_properties();

	if (m_createInfo.storage_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
		Utility::log().location().format("Not enough bindless storage buffers {} | {}", m_createInfo.storage_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
		m_createInfo.storage_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
	}	
	if (m_createInfo.uniform_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
		Utility::log().location().format("Not enough bindless uniform buffers {} | {}", m_createInfo.uniform_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
		m_createInfo.uniform_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
	}
	if (m_createInfo.sampled_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
		Utility::log().location().format("Not enough bindless sampled images {} | {}", m_createInfo.sampled_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages);
		m_createInfo.sampled_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages;
	}
	if (m_createInfo.storage_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages) {
		Utility::log().location().format("Not enough bindless storage images {} | {}", m_createInfo.storage_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages);
		m_createInfo.storage_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages;
	}
	if (m_createInfo.sampler_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
		Utility::log().location().format("Not enough bindless samplers {} | {}", m_createInfo.sampler_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers);
		m_createInfo.sampler_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers;
	}
	if (m_createInfo.acceleration_structure_count > rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
		Utility::log().location().format("Not enough bindless acceleration structures {} | {}", m_createInfo.acceleration_structure_count, rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures);
		m_createInfo.acceleration_structure_count = rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures;
	}


	std::vector bindings{
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
	};

	
	std::vector poolSizes{
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
		}
	};
	if (m_createInfo.acceleration_structure_count) {
		bindings.push_back(VkDescriptorSetLayoutBinding
			{
				.binding = accelerationStructureBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(accelerationStructureBinding),
				.descriptorCount = m_createInfo.acceleration_structure_count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			});
		poolSizes.push_back(VkDescriptorPoolSize{
			.type = bindings[5].descriptorType,
			.descriptorCount = bindings[5].descriptorCount
			});
	}
	std::vector<VkDescriptorBindingFlags> flags;

	VkDescriptorBindingFlags flag{0};
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingPartiallyBound) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingUpdateUnusedWhilePending) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
	}
	flags.resize(bindings.size(),flag);
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingStorageBufferUpdateAfterBind) {
		flags[storageBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingUniformBufferUpdateAfterBind) {
		flags[uniformBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingSampledImageUpdateAfterBind) {
		flags[samplerBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		flags[sampledImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (r_device.get_physical_device().get_vulkan12_features().descriptorBindingStorageImageUpdateAfterBind) {
		flags[storageImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}

	if (m_createInfo.acceleration_structure_count) {
		if (r_device.get_physical_device().get_acceleration_structure_features().descriptorBindingAccelerationStructureUpdateAfterBind) {
			flags[accelerationStructureBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
	}

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

	if (auto result = r_device.get_device().vkCreateDescriptorSetLayout( &setLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}

	VkDescriptorPoolCreateInfo poolCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
		.maxSets = 1,
		.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
		.pPoolSizes = poolSizes.data(),
	};
	if (auto result = r_device.get_device().vkCreateDescriptorPool(&poolCreateInfo, r_device.get_allocator(), &m_handle); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
	}

}

vulkan::DescriptorPool::~DescriptorPool() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_device.get_device().vkDestroyDescriptorPool(m_handle, r_device.get_allocator());
	if (m_layout != VK_NULL_HANDLE)
		r_device.get_device().vkDestroyDescriptorSetLayout(m_layout, r_device.get_allocator());
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
		.descriptorPool = m_handle,
		.descriptorSetCount = 1,
		.pSetLayouts = &m_layout
	};
	if (auto result = r_device.get_device().vkAllocateDescriptorSets( &allocateInfo, &set); result != VK_SUCCESS) {
		throw Utility::VulkanException(result);
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
