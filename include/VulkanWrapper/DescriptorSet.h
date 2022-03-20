#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
#include "Shader.h"
#include <Util>
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
	constexpr uint32_t storageBufferBinding = 0;
	constexpr uint32_t uniformBufferBinding = 1;
	constexpr uint32_t samplerBinding = 2;
	constexpr uint32_t sampledImageBinding = 3;
	constexpr uint32_t storageImageBinding = 4;
	constexpr uint32_t accelerationStructureBinding = 5;
	struct DescriptorCreateInfo {
		uint32_t storage_buffer_count = 512 * 1024;
		uint32_t uniform_buffer_count = 1 * 1024;
		uint32_t sampler_count = 4 * 1024;
		uint32_t sampled_image_count = 512 * 1024;
		uint32_t storage_image_count = 64 * 1024;
		uint32_t acceleration_structure_count = 32 * 1024;
	};
	class DescriptorPool;
	class DescriptorSet {
	public:
		DescriptorSet(DescriptorPool& pool, VkDescriptorSet vkHandle);
		//void set_tex(binding, tex);
		uint32_t set_storage_buffer(const VkDescriptorBufferInfo& bufferInfo);
		uint32_t set_uniform_buffer(const VkDescriptorBufferInfo& bufferInfo);
		uint32_t set_sampler(const VkDescriptorImageInfo& imageInfo);
		uint32_t set_sampled_image(const VkDescriptorImageInfo& imageInfo);
		uint32_t set_storage_image(const VkDescriptorImageInfo& imageInfo);
		uint32_t set_acceleration_structure(const VkAccelerationStructureKHR& accelerationStructure);
		std::vector<uint32_t> set_storage_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos);
		std::vector<uint32_t> set_uniform_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos);
		std::vector<uint32_t> set_sampler(const std::vector<VkDescriptorImageInfo>& imageInfos);
		std::vector<uint32_t> set_sampled_image(const std::vector<VkDescriptorImageInfo>& imageInfos);
		std::vector<uint32_t> set_storage_image(const std::vector<VkDescriptorImageInfo>& imageInfos);
		std::vector<uint32_t> set_acceleration_structure(const std::vector<VkAccelerationStructureKHR>& accelerationStructures);
		void free_storage_buffer(uint32_t set);
		void free_uniform_buffer(uint32_t set);
		void free_sampler(uint32_t set);
		void free_sampled_image(uint32_t set);
		void free_storage_image(uint32_t set);
		void free_acceleration_structure(uint32_t set);
		operator VkDescriptorSet() const noexcept;
		VkDescriptorSet get_set() const noexcept;
		static VkDescriptorType bindless_binding_to_type(uint32_t) noexcept;
	private:
		struct Write {
			enum class Type {
				Image,
				Buffer,
				AccelerationStructure
			} type;
			uint32_t offset;
			uint32_t count;
			uint32_t binding;
			uint32_t arrayElement;
		};
		DescriptorPool& r_pool;
		VkDescriptorSet m_vkHandle;
		std::vector<uint32_t> m_storageBufferFsi;
		std::vector<uint32_t> m_uniformBufferFsi;
		std::vector<uint32_t> m_samplerFsi;
		std::vector<uint32_t> m_sampledImageFsi;
		std::vector<uint32_t> m_storageImageFsi;
		std::vector<uint32_t> m_accelerationStructureFsi;
		uint32_t m_storageBufferCount {0};
		uint32_t m_uniformBufferCount {0};
		uint32_t m_samplerCount {0};
		uint32_t m_sampledImageCount {0};
		uint32_t m_storageImageCount {0};
		uint32_t m_accelerationStructureCount {0};
	};
	class DescriptorPool {
	public:
		friend class DescriptorSet;
		DescriptorPool(LogicalDevice& device, const DescriptorCreateInfo& createInfo = {});
		~DescriptorPool() noexcept;
		DescriptorPool(DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&&) = delete;
		DescriptorPool& operator=(DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&&) = delete;

		//void get_set()
		DescriptorSet allocate_set();
		const DescriptorCreateInfo& get_info() const;
		VkDescriptorSetLayout get_layout() const;
	private:
		LogicalDevice& r_device;
		VkDescriptorPool m_pool;
		VkDescriptorSetLayout m_layout;
		DescriptorCreateInfo m_createInfo;
	};

	
}
#endif