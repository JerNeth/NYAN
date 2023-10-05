#include "VulkanWrapper/DescriptorPool.hpp"

#include "descriptors.h"

#include "Utility/Log.h"
#include "Utility/Exceptions.h"

#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/DescriptorSet.h"

namespace vulkan
{
	DescriptorPool::DescriptorPool(LogicalDevice& device) :
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
				VkDescriptorPoolSize{
					.type = bindings[1].descriptorType,
					.descriptorCount = bindings[1].descriptorCount
			},
				VkDescriptorPoolSize{
					.type = bindings[2].descriptorType,
					.descriptorCount = bindings[2].descriptorCount
			},
				VkDescriptorPoolSize{
					.type = bindings[3].descriptorType,
					.descriptorCount = bindings[3].descriptorCount
			},
				VkDescriptorPoolSize{
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

		if (auto result = r_device.get_device().vkCreateDescriptorSetLayout(&setLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
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

	DescriptorPool::DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo) :
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
				VkDescriptorPoolSize{
					.type = bindings[1].descriptorType,
					.descriptorCount = bindings[1].descriptorCount
			},
				VkDescriptorPoolSize{
					.type = bindings[2].descriptorType,
					.descriptorCount = bindings[2].descriptorCount
			},
				VkDescriptorPoolSize{
					.type = bindings[3].descriptorType,
					.descriptorCount = bindings[3].descriptorCount
			},
				VkDescriptorPoolSize{
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

		if (auto result = r_device.get_device().vkCreateDescriptorSetLayout(&setLayoutCreateInfo, r_device.get_allocator(), &m_layout); result != VK_SUCCESS) {
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

	DescriptorPool::~DescriptorPool() noexcept
	{
		if (m_handle != VK_NULL_HANDLE)
			r_device.get_device().vkDestroyDescriptorPool(m_handle, r_device.get_allocator());
		if (m_layout != VK_NULL_HANDLE)
			r_device.get_device().vkDestroyDescriptorSetLayout(m_layout, r_device.get_allocator());
	}


	DescriptorPool::DescriptorPool(DescriptorPool&& other) noexcept :
		VulkanObject(std::move(other)),
		m_layout(other.m_layout),
		m_createInfo(other.m_createInfo)
	{
		other.m_layout = VK_NULL_HANDLE;
	}

		vulkan::DescriptorPool& vulkan::DescriptorPool::operator=(DescriptorPool&& other) noexcept
	{
		if (this != &other)
		{
			assert(&r_device == &other.r_device);
			m_handle = other.m_handle;
			other.m_handle = VK_NULL_HANDLE;
			m_layout = other.m_layout;
			other.m_layout = VK_NULL_HANDLE;
			m_createInfo = other.m_createInfo;
		}
		return *this;
	}

	DescriptorSet DescriptorPool::allocate_set()
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
		if (auto result = r_device.get_device().vkAllocateDescriptorSets(&allocateInfo, &set); result != VK_SUCCESS) {
			throw Utility::VulkanException(result);
		}
		return { *this, set };
	}
	const DescriptorCreateInfo& DescriptorPool::get_info() const
	{
		return m_createInfo;
	}

	VkDescriptorSetLayout DescriptorPool::get_layout() const
	{
		return m_layout;
	}
}