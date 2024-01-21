#include "VulkanWrapper/DescriptorPool.hpp"

#include "descriptors.h"

#include "Utility/Log.h"
#include "Utility/Exceptions.h"

#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"
#include "VulkanWrapper/DescriptorSet.h"

namespace vulkan
{
	DescriptorPool::DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo,
		VkDescriptorSetLayout layout, VkDescriptorPool pool) noexcept :
		VulkanObject(device, pool),
		m_layout(layout),
		m_createInfo(createInfo)
	{
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

	std::expected<DescriptorSet, vulkan::Error> DescriptorPool::allocate_set() const noexcept
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
			return std::unexpected{ vulkan::Error{result} };
		}
		return DescriptorSet( *this, set );
	}
	const DescriptorCreateInfo& DescriptorPool::get_info() const noexcept
	{
		return m_createInfo;
	}

	VkDescriptorSetLayout DescriptorPool::get_layout() const noexcept
	{
		return m_layout;
	}

	std::expected<DescriptorPool, vulkan::Error> DescriptorPool::create_descriptor_pool(LogicalDevice& device,
		DescriptorCreateInfo createInfo) noexcept
	{

		const auto& vulkan12Properties = device.get_physical_device().get_vulkan12_properties();
		const auto& rtProperties = device.get_physical_device().get_acceleration_structure_properties();

		if (createInfo.storage_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers) {
			Utility::Logger::warning().location().format("Not enough bindless storage buffers {} | {}", createInfo.storage_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
			createInfo.storage_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers;
		}
		if (createInfo.uniform_buffer_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers) {
			Utility::Logger::warning().location().format("Not enough bindless uniform buffers {} | {}", createInfo.uniform_buffer_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
			createInfo.uniform_buffer_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers;
		}
		if (createInfo.sampled_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages) {
			Utility::Logger::warning().location().format("Not enough bindless sampled images {} | {}", createInfo.sampled_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages);
			createInfo.sampled_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages;
		}
		if (createInfo.storage_image_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages) {
			Utility::Logger::warning().location().format("Not enough bindless storage images {} | {}", createInfo.storage_image_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages);
			createInfo.storage_image_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages;
		}
		if (createInfo.sampler_count > vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers) {
			Utility::Logger::warning().location().format("Not enough bindless samplers {} | {}", createInfo.sampler_count, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers);
			createInfo.sampler_count = vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers;
		}
		if (createInfo.acceleration_structure_count > rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures) {
			Utility::Logger::warning().location().format("Not enough bindless acceleration structures {} | {}", createInfo.acceleration_structure_count, rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures);
			createInfo.acceleration_structure_count = rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures;
		}


		std::vector bindings{
			VkDescriptorSetLayoutBinding
			{
				.binding = storageBufferBinding,
					.descriptorType = DescriptorSet::bindless_binding_to_type(storageBufferBinding),
					.descriptorCount = createInfo.storage_buffer_count,
					.stageFlags = VK_SHADER_STAGE_ALL,
					.pImmutableSamplers = nullptr
			},
				VkDescriptorSetLayoutBinding
			{
				.binding = uniformBufferBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(uniformBufferBinding),
				.descriptorCount = createInfo.uniform_buffer_count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			},
				VkDescriptorSetLayoutBinding
			{
				.binding = samplerBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(samplerBinding),
				.descriptorCount = createInfo.sampler_count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			},
				VkDescriptorSetLayoutBinding
			{
				.binding = sampledImageBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(sampledImageBinding),
				.descriptorCount = createInfo.sampled_image_count,
				.stageFlags = VK_SHADER_STAGE_ALL,
				.pImmutableSamplers = nullptr
			},
				VkDescriptorSetLayoutBinding
			{
				.binding = storageImageBinding,
				.descriptorType = DescriptorSet::bindless_binding_to_type(storageImageBinding),
				.descriptorCount = createInfo.storage_image_count,
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
		if (createInfo.acceleration_structure_count) {
			bindings.push_back(VkDescriptorSetLayoutBinding
				{
					.binding = accelerationStructureBinding,
					.descriptorType = DescriptorSet::bindless_binding_to_type(accelerationStructureBinding),
					.descriptorCount = createInfo.acceleration_structure_count,
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
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingPartiallyBound) {
			//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			flag |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
		}
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingUpdateUnusedWhilePending) {
			//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
			flag |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
		}
		flags.resize(bindings.size(), flag);
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingStorageBufferUpdateAfterBind) {
			flags[storageBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingUniformBufferUpdateAfterBind) {
			flags[uniformBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingSampledImageUpdateAfterBind) {
			flags[samplerBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
			flags[sampledImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
		if (device.get_physical_device().get_vulkan12_features().descriptorBindingStorageImageUpdateAfterBind) {
			flags[storageImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}

		if (createInfo.acceleration_structure_count) {
			if (device.get_physical_device().get_acceleration_structure_features().descriptorBindingAccelerationStructureUpdateAfterBind) {
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
		VkDescriptorSetLayout layout {VK_NULL_HANDLE};
		if (auto result = device.get_device().vkCreateDescriptorSetLayout(&setLayoutCreateInfo, device.get_allocator(), &layout); result != VK_SUCCESS) {
			return std::unexpected{ vulkan::Error{result} };
		}

		VkDescriptorPoolCreateInfo poolCreateInfo{
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
			.maxSets = 1,
			.poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
			.pPoolSizes = poolSizes.data(),
		};
		VkDescriptorPool pool{ VK_NULL_HANDLE };
		if (auto result = device.get_device().vkCreateDescriptorPool(&poolCreateInfo, device.get_allocator(), &pool); result != VK_SUCCESS) {
			device.get_deletion_queue().queue_descriptor_set_layout_deletion(layout);
			return std::unexpected{ vulkan::Error{result} };
		}

		return DescriptorPool( device, createInfo, layout, pool);
	}

	std::expected<DescriptorPool, vulkan::Error> DescriptorPool::create_descriptor_pool(LogicalDevice& device) noexcept
	{
		return create_descriptor_pool(device, DescriptorCreateInfo{
			.storage_buffer_count {SSBO_COUNT},
			.uniform_buffer_count {UBO_COUNT},
			.sampler_count {SAMPLER_COUNT},
			.sampled_image_count {TEXTURE_COUNT},
			.storage_image_count {IMAGE_COUNT},
			.acceleration_structure_count {ACC_COUNT},
			});
	}

}
