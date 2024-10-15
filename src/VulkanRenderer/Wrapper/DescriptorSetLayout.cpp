module;

//#include <array>
//#include <cassert>
//#include <expected>
//#include <utility>
//#include <string_view>

#include "volk.h"

module NYANVulkan;
import std;

import NYANLog;

using namespace nyan::vulkan;

DescriptorSetLayout::DescriptorSetLayout(DescriptorSetLayout&& other) noexcept :
	Object(*other.ptr_device, other.m_handle),
	r_deletionQueue(other.r_deletionQueue),
	m_info(other.m_info)
{
	other.m_handle = VK_NULL_HANDLE;
}

DescriptorSetLayout& DescriptorSetLayout::operator=(DescriptorSetLayout&& other) noexcept
{
	if(this != std::addressof(other))
	{
		::assert(ptr_device == other.ptr_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_info, other.m_info);
	}
	return *this;
}

DescriptorSetLayout::~DescriptorSetLayout() noexcept
{
	if (m_handle != VK_NULL_HANDLE)
		r_deletionQueue.queue_deletion(m_handle);
}

const DescriptorSetLayout::DescriptorInfo& DescriptorSetLayout::get_info() const noexcept
{
	return m_info;
}

VkDescriptorType DescriptorSetLayout::bindless_binding_to_type(const uint32_t binding) noexcept
{
	switch (binding) {
	case DescriptorSetLayout::storageBufferBinding:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	case DescriptorSetLayout::uniformBufferBinding:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	case DescriptorSetLayout::samplerBinding:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
	case DescriptorSetLayout::sampledImageBinding:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	case DescriptorSetLayout::storageImageBinding:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	case DescriptorSetLayout::inputAttachmentBinding:
		return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
	case DescriptorSetLayout::accelerationStructureBinding:
		return VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	default:
		::assert(false);
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	}
}

std::expected<DescriptorSetLayout, Error> DescriptorSetLayout::create(LogicalDevice& device, DescriptorInfo info) noexcept
{
	const auto& physicalDevice = device.get_physical_device();
	const auto& vulkan12Properties = physicalDevice.get_vulkan12_properties();
	const auto& rtProperties = physicalDevice.get_acceleration_structure_properties();


	//Not sure if validation instead of error is better here.
	auto validateDescriptorCount = [&](std::string_view message, auto& info, auto maxVal)
		{
			if (info > maxVal) [[unlikely]] {
				log::warning().format(message, info, maxVal);
				info = maxVal;
			}
		};

	validateDescriptorCount("Device does not support enough bindless storage buffers {} | {}", info.storageBufferCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageBuffers);
	validateDescriptorCount("Device does not support enough bindless uniform buffers {} | {}", info.uniformBufferCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindUniformBuffers);
	validateDescriptorCount("Device does not support enough bindless sampled images {} | {}", info.sampledImageCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSampledImages);
	validateDescriptorCount("Device does not support enough bindless storage images  {} | {}", info.storageImageCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindStorageImages);
	validateDescriptorCount("Device does not support enough bindless bindless samplers {} | {}", info.samplerCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindSamplers);
	validateDescriptorCount("Device does not support enough bindless input attachments {} | {}", info.inputAttachmentCount, vulkan12Properties.maxPerStageDescriptorUpdateAfterBindInputAttachments);
	validateDescriptorCount("Device does not support enough bindless acceleration structures {} | {}", info.accelerationStructureCount, rtProperties.maxDescriptorSetUpdateAfterBindAccelerationStructures);

	auto descriptorSum = info.storageBufferCount + info.uniformBufferCount + info.sampledImageCount + info.storageImageCount + info.samplerCount + info.inputAttachmentCount + info.accelerationStructureCount;
	if (descriptorSum > vulkan12Properties.maxPerStageUpdateAfterBindResources) [[unlikely]] {
		log::error().format("Not enough bindless descriptors {} | {}", descriptorSum, vulkan12Properties.maxPerStageUpdateAfterBindResources);
		return std::unexpected{ Error{VK_ERROR_UNKNOWN} };
	}

	std::array bindings{
		VkDescriptorSetLayoutBinding
		{
			.binding = storageBufferBinding,
			.descriptorType = bindless_binding_to_type(storageBufferBinding),
			.descriptorCount = info.storageBufferCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = uniformBufferBinding,
			.descriptorType = bindless_binding_to_type(uniformBufferBinding),
			.descriptorCount = info.uniformBufferCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = samplerBinding,
			.descriptorType = bindless_binding_to_type(samplerBinding),
			.descriptorCount = info.samplerCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = sampledImageBinding,
			.descriptorType = bindless_binding_to_type(sampledImageBinding),
			.descriptorCount = info.sampledImageCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = storageImageBinding,
			.descriptorType = bindless_binding_to_type(storageImageBinding),
			.descriptorCount = info.storageImageCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = inputAttachmentBinding,
			.descriptorType = bindless_binding_to_type(inputAttachmentBinding),
			.descriptorCount = info.inputAttachmentCount,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = accelerationStructureBinding,
			.descriptorType = bindless_binding_to_type(accelerationStructureBinding),
			.descriptorCount = info.accelerationStructureCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		}
	};
	if (!device.get_enabled_extensions().accelerationStructure)
		bindings[6].descriptorCount = 0;

	uint32_t bindingCount = 0;
	for (uint32_t i = 0; i < bindings.size(); ++i)
		if (bindings[i].descriptorCount)
			bindingCount = i + 1;
		

	VkDescriptorBindingFlags flag{ 0 };

	const auto& vulkan12Features = physicalDevice.get_vulkan12_features();

	if (vulkan12Features.descriptorBindingPartiallyBound) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT;
	}
	if (vulkan12Features.descriptorBindingUpdateUnusedWhilePending) {
		//	//VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT |
		flag |= VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT;
	}

	std::array<VkDescriptorBindingFlags, bindings.size()> flags;
	flags.fill(flag);

	if (vulkan12Features.descriptorBindingStorageBufferUpdateAfterBind) {
		flags[storageBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (vulkan12Features.descriptorBindingUniformBufferUpdateAfterBind) {
		flags[uniformBufferBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}
	if (vulkan12Features.descriptorBindingSampledImageUpdateAfterBind) {
		flags[samplerBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		flags[sampledImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT; 
	}
	if (vulkan12Features.descriptorBindingStorageImageUpdateAfterBind) {
		flags[storageImageBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
	}

	if (info.accelerationStructureCount) {
		if (physicalDevice.get_acceleration_structure_features().descriptorBindingAccelerationStructureUpdateAfterBind) {
			flags[accelerationStructureBinding] |= VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;
		}
	}

	VkDescriptorSetLayoutBindingFlagsCreateInfo flagsCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
		.bindingCount = bindingCount,
		.pBindingFlags = flags.data()
	};

	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = &flagsCreateInfo,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
		.bindingCount = bindingCount,
		.pBindings = bindings.data()
	};
	const auto& deviceWrapper = device.get_device();

	VkDescriptorSetLayout layout{ VK_NULL_HANDLE };

	if (auto result = deviceWrapper.vkCreateDescriptorSetLayout(&setLayoutCreateInfo, &layout); result != VK_SUCCESS) [[unlikely]] 
		return std::unexpected{ Error{result} };

	return DescriptorSetLayout{ deviceWrapper, layout, device.get_deletion_queue(), std::move(info)};
}

DescriptorSetLayout::DescriptorSetLayout(const LogicalDeviceWrapper& deviceWrapper,
	VkDescriptorSetLayout layout, DeletionQueue& deletionQueue, DescriptorInfo info) noexcept :
	Object(deviceWrapper, layout),
	r_deletionQueue(deletionQueue),
	m_info(std::move(info))
{
	::assert(m_handle != VK_NULL_HANDLE);
}

std::expected<PushDescriptorSetLayout, Error> nyan::vulkan::PushDescriptorSetLayout::create(LogicalDevice& device, DescriptorInfo info) noexcept
{
	if (!device.get_enabled_extensions().pushDescriptors) [[unlikely]]
		return std::unexpected{ Error{VK_ERROR_EXTENSION_NOT_PRESENT} };

	const auto& physicalDevice = device.get_physical_device();
	const auto& limits = physicalDevice.get_properties().limits;
	const auto& vulkan12Properties = physicalDevice.get_vulkan12_properties();
	const auto& rtProperties = physicalDevice.get_acceleration_structure_properties();
	const auto& pushDescriptorProperties = physicalDevice.get_push_descriptor_properties();


	//Not sure if validation instead of error is better here.
	auto validateDescriptorCount = [&](std::string_view message, auto& info, auto maxVal)
		{
			if (info > maxVal) {
				log::warning().format(message, info, maxVal);
				info = maxVal;
			}
		};

	validateDescriptorCount("Not enough descriptor storage buffers {} | {}", info.storageBufferCount, limits.maxPerStageDescriptorStorageBuffers);
	validateDescriptorCount("Not enough descriptor uniform buffers {} | {}", info.uniformBufferCount, limits.maxPerStageDescriptorUniformBuffers);
	validateDescriptorCount("Not enough descriptor sampled images {} | {}", info.sampledImageCount, limits.maxPerStageDescriptorSampledImages);
	validateDescriptorCount("Not enough descriptor storage images  {} | {}", info.storageImageCount, limits.maxPerStageDescriptorStorageImages);
	validateDescriptorCount("Not enough descriptor bindless samplers {} | {}", info.samplerCount, limits.maxPerStageDescriptorSamplers);
	validateDescriptorCount("Not enough descriptor input attachements {} | {}", info.inputAttachmentCount, limits.maxPerStageDescriptorInputAttachments);
	validateDescriptorCount("Not enough descriptor acceleration structures {} | {}", info.accelerationStructureCount, rtProperties.maxPerStageDescriptorAccelerationStructures);
	auto descriptorSum = info.storageBufferCount + info.uniformBufferCount + info.sampledImageCount + info.storageImageCount + info.samplerCount + info.inputAttachmentCount + info.accelerationStructureCount;
	if (descriptorSum > pushDescriptorProperties.maxPushDescriptors) [[unlikely]] {
		log::error().format("Not enough push descriptors {} | {}", descriptorSum, pushDescriptorProperties.maxPushDescriptors);
		return std::unexpected{ Error{VK_ERROR_UNKNOWN} };
	}
	if (descriptorSum > limits.maxPerStageResources) [[unlikely]] {
		log::error().format("Not enough descriptors {} | {}", descriptorSum, limits.maxPerStageResources);
		return std::unexpected{ Error{VK_ERROR_UNKNOWN} };
	}



	std::array bindings{
		VkDescriptorSetLayoutBinding
		{
			.binding = storageBufferBinding,
			.descriptorType = bindless_binding_to_type(storageBufferBinding),
			.descriptorCount = info.storageBufferCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = uniformBufferBinding,
			.descriptorType = bindless_binding_to_type(uniformBufferBinding),
			.descriptorCount = info.uniformBufferCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = samplerBinding,
			.descriptorType = bindless_binding_to_type(samplerBinding),
			.descriptorCount = info.samplerCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = sampledImageBinding,
			.descriptorType = bindless_binding_to_type(sampledImageBinding),
			.descriptorCount = info.sampledImageCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = storageImageBinding,
			.descriptorType = bindless_binding_to_type(storageImageBinding),
			.descriptorCount = info.storageImageCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = inputAttachmentBinding,
			.descriptorType = bindless_binding_to_type(inputAttachmentBinding),
			.descriptorCount = info.inputAttachmentCount,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
			.pImmutableSamplers = nullptr
		},
		VkDescriptorSetLayoutBinding
		{
			.binding = accelerationStructureBinding,
			.descriptorType = bindless_binding_to_type(accelerationStructureBinding),
			.descriptorCount = info.accelerationStructureCount,
			.stageFlags = VK_SHADER_STAGE_ALL,
			.pImmutableSamplers = nullptr
		}
	};
	if (!device.get_enabled_extensions().accelerationStructure)
		bindings[bindings.size() - 1].descriptorCount = 0;

	uint32_t bindingCount = 0;
	for (uint32_t i = 0; i < bindings.size(); ++i)
		if (bindings[i].descriptorCount)
			bindingCount = i + 1;


	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo{
		.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		.pNext = nullptr,
		.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR ,
		.bindingCount = bindingCount,
		.pBindings = bindings.data()
	};
	const auto& deviceWrapper = device.get_device();
	VkDescriptorSetLayout layout{ VK_NULL_HANDLE };
	if (auto result = deviceWrapper.vkCreateDescriptorSetLayout(&setLayoutCreateInfo, &layout); result != VK_SUCCESS) [[unlikely]] 
		return std::unexpected{ Error{result} };

	return PushDescriptorSetLayout{ deviceWrapper, layout, device.get_deletion_queue(), std::move(info) };
}

PushDescriptorSetLayout::PushDescriptorSetLayout(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSetLayout layout, DeletionQueue& deletionQueue, DescriptorInfo info) noexcept :
	DescriptorSetLayout(deviceWrapper, layout, deletionQueue, std::move(info))
{

}