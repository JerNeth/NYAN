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
		std::swap(m_handle, other.m_handle);
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

vulkan::Sampler::Sampler(LogicalDevice& device, VkSampler handle) noexcept :
	VulkanObject(device, handle)
{
}

vulkan::SamplerStorage::SamplerStorage(LogicalDevice& device) noexcept :
	r_device(device)
{
}

vulkan::Sampler& vulkan::SamplerStorage::get_sampler(DefaultSampler samplerType) noexcept
{
	assert(samplerType < DefaultSampler::Size);
	return m_samplers[static_cast<size_t>(samplerType)];
}

const vulkan::Sampler& vulkan::SamplerStorage::get_sampler(DefaultSampler samplerType) const noexcept
{
	assert(samplerType < DefaultSampler::Size);
	return m_samplers[static_cast<size_t>(samplerType)];
}

std::expected<void, vulkan::Error> vulkan::SamplerStorage::create_default_samplers() noexcept
{
	VkSamplerCreateInfo createInfo{
	.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO
	};
	createInfo.maxLod = VK_LOD_CLAMP_NONE;
	for (int i = 0; i < static_cast<int>(DefaultSampler::Size); i++) {
		auto type = static_cast<DefaultSampler>(i);
		switch (type) {
		case DefaultSampler::NearestShadow:
		case DefaultSampler::LinearShadow:
			createInfo.compareEnable = VK_TRUE;
			createInfo.compareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			break;
		default:
			createInfo.compareEnable = VK_FALSE;
			break;
		}

		switch (type) {
		case DefaultSampler::Anisotropic2Wrap:
		case DefaultSampler::Anisotropic2Clamp:
		case DefaultSampler::Anisotropic4Wrap:
		case DefaultSampler::Anisotropic4Clamp:
		case DefaultSampler::Anisotropic8Wrap:
		case DefaultSampler::Anisotropic8Clamp:
		case DefaultSampler::Anisotropic16Wrap:
		case DefaultSampler::Anisotropic16Clamp:
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::TrilinearWrap:
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			break;
		default:
			createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
			break;
		}
		switch (type) {
		case DefaultSampler::Anisotropic2Wrap:
		case DefaultSampler::Anisotropic2Clamp:
		case DefaultSampler::Anisotropic4Wrap:
		case DefaultSampler::Anisotropic4Clamp:
		case DefaultSampler::Anisotropic8Wrap:
		case DefaultSampler::Anisotropic8Clamp:
		case DefaultSampler::Anisotropic16Wrap:
		case DefaultSampler::Anisotropic16Clamp:
			createInfo.anisotropyEnable = VK_TRUE;
			break;
		default:
			createInfo.anisotropyEnable = VK_FALSE;
			break;
		}

		switch (type) {
		case DefaultSampler::Anisotropic2Wrap:
		case DefaultSampler::Anisotropic2Clamp:
		case DefaultSampler::Anisotropic4Wrap:
		case DefaultSampler::Anisotropic4Clamp:
		case DefaultSampler::Anisotropic8Wrap:
		case DefaultSampler::Anisotropic8Clamp:
		case DefaultSampler::Anisotropic16Wrap:
		case DefaultSampler::Anisotropic16Clamp:
		case DefaultSampler::LinearClamp:
		case DefaultSampler::LinearWrap:
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::TrilinearWrap:
		case DefaultSampler::LinearShadow:
			createInfo.magFilter = VK_FILTER_LINEAR;
			createInfo.minFilter = VK_FILTER_LINEAR;
			break;
		default:
			createInfo.magFilter = VK_FILTER_NEAREST;
			createInfo.minFilter = VK_FILTER_NEAREST;
			break;
		}

		switch (type) {
		case DefaultSampler::Anisotropic2Wrap:
		case DefaultSampler::Anisotropic2Clamp:
			createInfo.maxAnisotropy = 2.0f;
			break;
		case DefaultSampler::Anisotropic4Wrap:
		case DefaultSampler::Anisotropic4Clamp:
			createInfo.maxAnisotropy = 4.0f;
			break;
		case DefaultSampler::Anisotropic8Wrap:
		case DefaultSampler::Anisotropic8Clamp:
			createInfo.maxAnisotropy = 8.0f;
			break;
		case DefaultSampler::Anisotropic16Wrap:
		case DefaultSampler::Anisotropic16Clamp:
			createInfo.maxAnisotropy = 16.0f;
			break;
		default:
			createInfo.maxAnisotropy = 1.0f;
			break;
		}

		switch (type) {
		case DefaultSampler::Anisotropic2Clamp:
		case DefaultSampler::Anisotropic4Clamp:
		case DefaultSampler::Anisotropic8Clamp:
		case DefaultSampler::Anisotropic16Clamp:
		case DefaultSampler::LinearShadow:
		case DefaultSampler::NearestShadow:
		case DefaultSampler::TrilinearClamp:
		case DefaultSampler::LinearClamp:
		case DefaultSampler::NearestClamp:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			break;
		case DefaultSampler::Anisotropic2Wrap:
		case DefaultSampler::Anisotropic4Wrap:
		case DefaultSampler::Anisotropic8Wrap:
		case DefaultSampler::Anisotropic16Wrap:
		case DefaultSampler::LinearWrap:
		case DefaultSampler::TrilinearWrap:
		case DefaultSampler::NearestWrap:
		default:
			createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			break;
		}
		if (auto sampler = Sampler::create(r_device, createInfo); sampler) {
			m_samplers.push_back(std::move(*sampler));
		}
		else
			return std::unexpected{ sampler.error() };
	}
	return {};
}

//vulkan::Sampler::Sampler(vulkan::LogicalDevice& device, const VkSampler handle) noexcept :
//	VulkanObject<VkSampler_T*>(device, handle)
//	//m_createInfo(createInfo)
//{
//
//}