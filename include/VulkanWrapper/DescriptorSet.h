#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
#include <Utility.h>
#include <stdexcept>
#include <string>
#include <bitset>
#include <array>
#include <vector>

namespace Vulkan {
	class LogicalDevice;
	
	struct DescriptorSetLayout {
		std::bitset<MAX_BINDINGS> imageSampler;
		std::bitset<MAX_BINDINGS> sampledBuffer;
		std::bitset<MAX_BINDINGS> storageImage;
		std::bitset<MAX_BINDINGS> uniformBuffer;
		std::bitset<MAX_BINDINGS> storageBuffer;
		std::bitset<MAX_BINDINGS> inputAttachment;
		std::bitset<MAX_BINDINGS> separateImage;
		std::bitset<MAX_BINDINGS> seperateSampler;
		std::bitset<MAX_BINDINGS> fp;
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