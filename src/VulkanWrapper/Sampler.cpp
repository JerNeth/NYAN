#include "VulkanWrapper/Sampler.hpp"

#include "LogicalDevice.h"

#include "VulkanWrapper/LogicalDevice.h"

vulkan::Sampler::~Sampler() noexcept{
	if (m_handle != VK_NULL_HANDLE) {
		r_device.get_deletion_queue().queue_sampler_deletion(m_handle);
		m_handle = VK_NULL_HANDLE;
	}
}

vulkan::Sampler::Sampler(Sampler&& other) noexcept :
	VulkanObject<VkSampler_T*>(other.r_device, other.m_handle)
	//m_createInfo(other.m_createInfo)
{
	other.m_handle = VK_NULL_HANDLE;
}

vulkan::Sampler& vulkan::Sampler::operator=(Sampler&& other) noexcept
{
	if(this != &other)
	{
		
	}
	return *this;
}

std::expected<vulkan::Sampler, vulkan::Error> vulkan::Sampler::create(vulkan::LogicalDevice& device, const VkSamplerCreateInfo& createInfo) noexcept
{
	VkSampler handle;
	if (const auto result = device.get_device().vkCreateSampler(&createInfo, device.get_allocator(), &handle); result != VK_SUCCESS) {
		return std::unexpected{vulkan::Error{result}};
	}
	return Sampler{device, handle};
}

vulkan::Sampler::Sampler(vulkan::LogicalDevice& device, const VkSampler handle) noexcept :
	VulkanObject<VkSampler_T*>(device, handle)
	//m_createInfo(createInfo)
{

}