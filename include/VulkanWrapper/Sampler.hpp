#ifndef VKSAMPLER_H
#define VKSAMPLER_H
#pragma once

#include <expected>

#include "VulkanWrapper/VulkanIncludes.h"

#include "VulkanWrapper/VulkanForwards.h"
#include "VulkanWrapper/VulkanObject.h"
#include "VulkanWrapper/VulkanError.hpp"

namespace vulkan {

	class Sampler : public VulkanObject<VkSampler> {
	public:
		~Sampler() noexcept;
		Sampler(Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&) = delete;
		Sampler& operator=(Sampler&&) noexcept;

		static std::expected<vulkan::Sampler, vulkan::Error> create(vulkan::LogicalDevice& device, const VkSamplerCreateInfo& createInfo) noexcept;
	private:
		Sampler(LogicalDevice& parent, VkSampler handle) noexcept;

		//VkSamplerCreateInfo m_createInfo;
	};
}

#endif //VKSAMPLER_H