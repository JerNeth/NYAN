#include "Sampler.h"
#include "LogicalDevice.h"
vulkan::Sampler::Sampler(vulkan::LogicalDevice& device, const VkSamplerCreateInfo& createInfo):
	r_device(device),
	m_createInfo(createInfo)
{

	if (auto result = vkCreateSampler(r_device.get_device(), &m_createInfo, r_device.get_allocator(), &m_vkHandle); result != VK_SUCCESS) {
		throw std::runtime_error("Could not create sampler");
	}
}
vulkan::Sampler::~Sampler() noexcept{
	if (m_vkHandle != VK_NULL_HANDLE) {
		r_device.queue_image_sampler_deletion(m_vkHandle);
		m_vkHandle = VK_NULL_HANDLE;
	}
}

vulkan::Sampler::Sampler(Sampler&& other) noexcept :
	r_device(other.r_device),
	m_createInfo(other.m_createInfo)
{
	if (this != &other) {
		std::swap(m_vkHandle, other.m_vkHandle);
	}
}
