#include "DescriptorSet.h"
#include "LogicalDevice.h"
Vulkan::DescriptorSetAllocator::DescriptorSetAllocator(LogicalDevice& parent, const DescriptorSetLayout& layout) :
r_parent(parent)
{
	
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	for (uint32_t i = 0; i < MAX_BINDINGS; i++) {
		if (layout.stages[i]) {
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
			}
			if (layout.sampler.test(i)) {
				bindings.push_back(VkDescriptorSetLayoutBinding
					{
						.binding = i,
						.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
						.descriptorCount = layout.arraySizes[i],
						.stageFlags = static_cast<VkShaderStageFlags>(layout.stages[i]),
						.pImmutableSamplers = nullptr
					}
				);
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
}


VkDescriptorSetLayout Vulkan::DescriptorSetAllocator::get_layout() const noexcept
{
	return m_layout;
}
