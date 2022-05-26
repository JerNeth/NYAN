#ifndef VKSAMPLER_H
#define VKSAMPLER_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"
#include <Util>

namespace vulkan {

	class Sampler : public Utility::UIDC {
	public:
		Sampler(LogicalDevice& parent, const VkSamplerCreateInfo& createInfo);
		explicit operator VkSampler() const noexcept { return m_vkHandle; }
		VkSampler get_handle() const noexcept { return m_vkHandle; }
		~Sampler() noexcept;
		Sampler(Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&) = delete;
		Sampler& operator=(Sampler&&) = delete;
	private:
		LogicalDevice& r_device;
		VkSampler m_vkHandle = VK_NULL_HANDLE;
		VkSamplerCreateInfo m_createInfo;
	};
}

#endif //VKSAMPLER_H