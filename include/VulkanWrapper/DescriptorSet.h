#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
#include "Shader.h"
#include "Utility.h"
namespace vulkan {
	class LogicalDevice;
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