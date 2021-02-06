#ifndef VKSAMPLER_H
#define VKSAMPLER_H
#pragma once
#include "VulkanIncludes.h"
#include "Utility.h"

namespace Vulkan {

	class LogicalDevice;
	enum class DefaultSampler {
		NearestClamp,
		LinearClamp,
		TrilinearClamp,
		NearestWrap,
		LinearWrap,
		TrilinearWrap,
		NearestShadow,
		LinearShadow,
		Size
	};
	struct SamplerCreateInfo {
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerMipmapMode mipmapMode;
		VkSamplerAddressMode addressModeU;
		VkSamplerAddressMode addressModeV;
		VkSamplerAddressMode addressModeW;
		float mipLodBias;
		VkBool32 anisotropyEnable;
		float maxAnisotropy;
		VkBool32 compareEnable;
		VkCompareOp compareOp;
		float minLod;
		float maxLod;
		VkBorderColor borderColor;
		VkBool32 unnormalizedCoordinates;
	};
	class Sampler : public Utility::UIDC{
	public:
		Sampler(LogicalDevice& parent, const SamplerCreateInfo& createInfo);
		explicit operator VkSampler() const noexcept { return m_vkHandle; }
		VkSampler get_handle() const noexcept { return m_vkHandle; }
		~Sampler() noexcept;
		Sampler(Sampler&) = delete;
		Sampler(Sampler&& other) noexcept;
		Sampler& operator=(Sampler&) = delete;
		Sampler& operator=(Sampler&&) = delete;
	private:
		LogicalDevice& parent;
		VkSampler m_vkHandle = VK_NULL_HANDLE;
		SamplerCreateInfo m_createInfo;
	};
}

#endif //VKSAMPLER_H