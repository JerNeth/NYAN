#include "VulkanWrapper/DescriptorSet.h"

#include <numeric>

#include "VulkanWrapper/LogicalDevice.h"
#include "descriptors.h"

vulkan::DescriptorSet::DescriptorSet(const DescriptorPool& pool, VkDescriptorSet vkHandle) noexcept :
	r_pool(pool),
	m_vkHandle(vkHandle)
{
}

uint32_t vulkan::DescriptorSet::set_storage_buffer(const VkDescriptorBufferInfo& bufferInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_uniform_buffer(const VkDescriptorBufferInfo& bufferInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets( 1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampler(const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_sampled_image(const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_storage_image(const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
	return arrayElement;
}

uint32_t vulkan::DescriptorSet::set_acceleration_structure(const VkAccelerationStructureKHR& accelerationStructure) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
	return arrayElement;
}
uint32_t vulkan::DescriptorSet::reserve_storage_buffer() noexcept
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
uint32_t vulkan::DescriptorSet::reserve_uniform_buffer() noexcept
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
uint32_t vulkan::DescriptorSet::reserve_sampler() noexcept
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
uint32_t vulkan::DescriptorSet::reserve_sampled_image() noexcept
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
uint32_t vulkan::DescriptorSet::reserve_storage_image() noexcept
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
uint32_t vulkan::DescriptorSet::reserve_acceleration_structure() noexcept
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
void vulkan::DescriptorSet::set_storage_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_uniform_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_sampler(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_sampled_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_storage_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
void vulkan::DescriptorSet::set_acceleration_structure(uint32_t idx, const VkAccelerationStructureKHR& accelerationStructure) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);
}
std::vector<uint32_t> vulkan::DescriptorSet::set_storage_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_uniform_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_sampler(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_sampled_image(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_storage_image(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

	return arrayElements;
}
std::vector<uint32_t> vulkan::DescriptorSet::set_acceleration_structure(const std::vector<VkAccelerationStructureKHR>& accelerationStructures) noexcept
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
	r_pool.get_device().get_device().vkUpdateDescriptorSets(1, &write, 0, nullptr);

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

constexpr VkDescriptorType vulkan::DescriptorSet::bindless_binding_to_type(const uint32_t binding) noexcept
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