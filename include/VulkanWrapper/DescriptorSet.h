#ifndef VKDESCRIPTORSET_H
#define VKDESCRIPTORSET_H
#pragma once
#include "VulkanIncludes.h"
#include "VulkanForwards.h"

#include <vector>
#include <format>
#include <expected>

#include "Util"

namespace vulkan {
	class DescriptorSet {
	public:
		DescriptorSet(const DescriptorPool& pool, VkDescriptorSet vkHandle) noexcept;
		//void set_tex(binding, tex);
		uint32_t set_storage_buffer(const VkDescriptorBufferInfo& bufferInfo) noexcept;
		uint32_t set_uniform_buffer(const VkDescriptorBufferInfo& bufferInfo) noexcept;
		uint32_t set_sampler(const VkDescriptorImageInfo& imageInfo) noexcept;
		uint32_t set_sampled_image(const VkDescriptorImageInfo& imageInfo) noexcept;
		uint32_t set_storage_image(const VkDescriptorImageInfo& imageInfo) noexcept;
		uint32_t set_acceleration_structure(const VkAccelerationStructureKHR& accelerationStructure) noexcept;

		uint32_t reserve_storage_buffer() noexcept;
		uint32_t reserve_uniform_buffer() noexcept;
		uint32_t reserve_sampler() noexcept;
		uint32_t reserve_sampled_image() noexcept;
		uint32_t reserve_storage_image() noexcept;
		uint32_t reserve_acceleration_structure() noexcept;

		void set_storage_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo) noexcept;
		void set_uniform_buffer(uint32_t idx, const VkDescriptorBufferInfo& bufferInfo) noexcept;
		void set_sampler(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept;
		void set_sampled_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept;
		void set_storage_image(uint32_t idx, const VkDescriptorImageInfo& imageInfo) noexcept;
		void set_acceleration_structure(uint32_t idx, const VkAccelerationStructureKHR& accelerationStructure) noexcept;
		std::vector<uint32_t> set_storage_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos) noexcept;
		std::vector<uint32_t> set_uniform_buffer(const std::vector<VkDescriptorBufferInfo>& bufferInfos) noexcept;
		std::vector<uint32_t> set_sampler(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept;
		std::vector<uint32_t> set_sampled_image(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept;
		std::vector<uint32_t> set_storage_image(const std::vector<VkDescriptorImageInfo>& imageInfos) noexcept;
		std::vector<uint32_t> set_acceleration_structure(const std::vector<VkAccelerationStructureKHR>& accelerationStructures) noexcept;
		void free_storage_buffer(uint32_t set) noexcept;
		void free_uniform_buffer(uint32_t set) noexcept;
		void free_sampler(uint32_t set) noexcept;
		void free_sampled_image(uint32_t set) noexcept;
		void free_storage_image(uint32_t set) noexcept;
		void free_acceleration_structure(uint32_t set) noexcept;
		operator VkDescriptorSet() const noexcept;
		VkDescriptorSet get_set() const noexcept;
		static constexpr VkDescriptorType bindless_binding_to_type(uint32_t) noexcept;
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
		const DescriptorPool& r_pool;
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
}
#endif