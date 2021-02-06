#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
#include "Utility.h"
namespace Vulkan {
	class LogicalDevice;
	enum class DefaultSampler;
	struct DescriptorSetLayout {
		Utility::bitset<MAX_BINDINGS> imageSampler;
		Utility::bitset<MAX_BINDINGS> sampledBuffer;
		Utility::bitset<MAX_BINDINGS> storageImage;
		Utility::bitset<MAX_BINDINGS> uniformBuffer;
		Utility::bitset<MAX_BINDINGS> storageBuffer;
		Utility::bitset<MAX_BINDINGS> inputAttachment;
		Utility::bitset<MAX_BINDINGS> separateImage;
		Utility::bitset<MAX_BINDINGS> seperateSampler;
		Utility::bitset<MAX_BINDINGS> fp;
		Utility::bitset<MAX_BINDINGS> immutableSampler;
		Utility::bitarray<DefaultSampler, MAX_BINDINGS> immutableSamplers;
		std::array<uint8_t, MAX_BINDINGS> arraySizes;
		std::array<uint32_t, MAX_BINDINGS> stages{};
		friend bool operator==(DescriptorSetLayout& left, DescriptorSetLayout& right) {
			return left.imageSampler == right.imageSampler &&
				left.sampledBuffer == right.sampledBuffer &&
				left.storageImage == right.storageImage &&
				left.uniformBuffer == right.uniformBuffer &&
				left.storageBuffer == right.storageBuffer &&
				left.inputAttachment == right.inputAttachment &&
				left.separateImage == right.separateImage &&
				left.seperateSampler == right.seperateSampler &&
				left.fp == right.fp &&
				left.arraySizes == right.arraySizes &&
				left.stages == right.stages;
		}
		friend bool operator==(const DescriptorSetLayout& left, const DescriptorSetLayout& right) {
			return left.imageSampler == right.imageSampler &&
				left.sampledBuffer == right.sampledBuffer &&
				left.storageImage == right.storageImage &&
				left.uniformBuffer == right.uniformBuffer &&
				left.storageBuffer == right.storageBuffer &&
				left.inputAttachment == right.inputAttachment &&
				left.separateImage == right.separateImage &&
				left.seperateSampler == right.seperateSampler &&
				left.fp == right.fp &&
				left.arraySizes == right.arraySizes &&
				left.stages == right.stages;
		}
	};

	class DescriptorSetAllocator {
	public:
		DescriptorSetAllocator(LogicalDevice& parent, const DescriptorSetLayout& layout);
		~DescriptorSetAllocator();
		std::pair<VkDescriptorSet, bool> find(unsigned threadId, Utility::HashValue hash);
		VkDescriptorSetLayout get_layout() const noexcept;
		void clear();
	private:
		struct PerThread {
			bool removeStale = false;
			std::vector<VkDescriptorPool> pools;
			std::vector<VkDescriptorSet> vacant;
			std::vector<Utility::HashValue> stale;
			Utility::HashMap<VkDescriptorSet> hashMap;
		};
		std::vector<std::unique_ptr<PerThread>> perThread;
		std::vector<VkDescriptorPoolSize> poolSizes;
		LogicalDevice& r_device;
		VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
		
	};
	
}
#endif