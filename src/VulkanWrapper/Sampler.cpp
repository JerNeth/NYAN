#include "Sampler.h"
#include "LogicalDevice.h"
Vulkan::Sampler::Sampler(Vulkan::LogicalDevice& parent, const Vulkan::SamplerCreateInfo& createInfo):
	parent(parent),
	m_createInfo(createInfo)
{
	VkSamplerCreateInfo vkCreateInfo{
		.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		.magFilter = m_createInfo.magFilter,
		.minFilter = m_createInfo.minFilter,
		.mipmapMode = m_createInfo.mipmapMode,
		.addressModeU = m_createInfo.addressModeU,
		.addressModeV = m_createInfo.addressModeV ,
		.addressModeW = m_createInfo.addressModeW,
		.mipLodBias = m_createInfo.mipLodBias,
		.anisotropyEnable = m_createInfo.anisotropyEnable,
		.maxAnisotropy = m_createInfo.maxAnisotropy,
		.compareEnable = m_createInfo.compareEnable,
		.compareOp = m_createInfo.compareOp,
		.minLod = m_createInfo.minLod,
		.maxLod = m_createInfo.maxLod,
		.borderColor = m_createInfo.borderColor,
		.unnormalizedCoordinates = m_createInfo.unnormalizedCoordinates,
	};
	if (auto result = vkCreateSampler(parent.m_device, &vkCreateInfo, parent.m_allocator, &m_vkHandle); result != VK_SUCCESS) {
		throw std::runtime_error("Could not create sampler");
	}
}
Vulkan::Sampler::~Sampler() noexcept{
	if (m_vkHandle != VK_NULL_HANDLE) {
		vkDestroySampler(parent.m_device, m_vkHandle, parent.m_allocator);
		m_vkHandle = VK_NULL_HANDLE;
	}
}

Vulkan::Sampler::Sampler(Sampler&& other) noexcept:
	parent(other.parent),
	m_vkHandle(other.m_vkHandle),
	m_createInfo(other.m_createInfo)
{
	other.m_vkHandle = VK_NULL_HANDLE;
}
