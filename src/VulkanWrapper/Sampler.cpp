#include "Sampler.h"
#include "LogicalDevice.h"
Vulkan::Sampler::Sampler(Vulkan::LogicalDevice& device, const Vulkan::SamplerCreateInfo& createInfo):
	r_device(device),
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
	if (auto result = vkCreateSampler(r_device.get_device(), &vkCreateInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		throw std::runtime_error("Could not create sampler");
	}
}
Vulkan::Sampler::~Sampler() noexcept{
	if (m_vkHandle != VK_NULL_HANDLE) {
		r_device.queue_image_sampler_deletion(m_vkHandle);
		m_vkHandle = VK_NULL_HANDLE;
	}
}

Vulkan::Sampler::Sampler(Sampler&& other) noexcept:
	r_device(other.r_device),
	m_vkHandle(other.m_vkHandle),
	m_createInfo(other.m_createInfo)
{
	other.m_vkHandle = VK_NULL_HANDLE;
}
