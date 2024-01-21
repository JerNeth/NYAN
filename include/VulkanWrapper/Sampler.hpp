#ifndef VKSAMPLER_H
#define VKSAMPLER_H
#pragma once

#include <expected>
#include <vector>

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
		Sampler& operator=(Sampler&& other) noexcept;

		static std::expected<vulkan::Sampler, vulkan::Error> create(vulkan::LogicalDevice& device, const VkSamplerCreateInfo& createInfo) noexcept;
	private:
		Sampler(LogicalDevice& device, VkSampler handle) noexcept;

		//VkSamplerCreateInfo m_createInfo;
	};

	class SamplerStorage
	{
	public:
		SamplerStorage(LogicalDevice& device) noexcept;
		[[nodiscard]] Sampler& get_sampler(DefaultSampler samplerType) noexcept;
		[[nodiscard]] const Sampler& get_sampler(DefaultSampler samplerType) const noexcept;
		[[nodiscard]] std::expected<void, vulkan::Error> create_default_samplers() noexcept;
	private:
		LogicalDevice& r_device;
		std::vector<Sampler> m_samplers;
	};
}

#endif //VKSAMPLER_H