#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
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
		std::bitset<MAX_BINDINGS> sampler;
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
				left.sampler == right.sampler &&
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
				left.sampler == right.sampler &&
				left.fp == right.fp &&
				left.arraySizes == right.arraySizes &&
				left.stages == right.stages;
		}
	};
	
	
	class DescriptorSetAllocator {
	public:
		DescriptorSetAllocator(LogicalDevice& parent, const DescriptorSetLayout& layout);
		~DescriptorSetAllocator();
		VkDescriptorSet find(unsigned threadId);
		VkDescriptorSetLayout get_layout() const noexcept;
	private:
		LogicalDevice& r_parent;
		VkDescriptorSetLayout m_layout = VK_NULL_HANDLE;
	};
	
}
#endif