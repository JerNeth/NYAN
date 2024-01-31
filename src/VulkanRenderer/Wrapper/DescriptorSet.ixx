module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:DescriptorSet;
import :LogicalDeviceWrapper;
import :DescriptorSetLayout;
import :Object;
import NYANData;

export namespace nyan::vulkan::wrapper
{
	class DescriptorSet : public Object<VkDescriptorSet>
	{
	private:
		struct Bitmaps
		{
			util::data::DynamicBitset storageBuffers;
			util::data::DynamicBitset uniformBuffers;
			util::data::DynamicBitset samplers;
			util::data::DynamicBitset sampledImages;
			util::data::DynamicBitset storageImages;
			util::data::DynamicBitset accelerationStructures;
		};
		struct Counts
		{
			uint32_t storageBuffers { 0 };
			uint32_t uniformBuffers { 0 };
			uint32_t samplers { 0 };
			uint32_t sampledImages { 0 };
			uint32_t storageImages { 0 };
			uint32_t accelerationStructures{ 0 };
		};
	public:
		DescriptorSet(DescriptorSet&) = delete;
		DescriptorSet& operator=(DescriptorSet&) = delete;

		DescriptorSet(DescriptorSet&&) noexcept;
		DescriptorSet& operator=(DescriptorSet&&) noexcept;

		[[nodiscard]] static std::expected< DescriptorSet, Error> create(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSet handle, const DescriptorSetLayout& layout) noexcept;
	private:
		DescriptorSet(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSet handle, const DescriptorSetLayout& layout, Bitmaps bitmaps) noexcept;
		const DescriptorSetLayout& r_layout;

		Bitmaps m_bitmaps;
		Counts m_counts;
	};
}
