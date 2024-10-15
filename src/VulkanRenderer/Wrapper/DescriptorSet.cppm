module;

//#include <expected>

#include "volk.h"

export module NYANVulkan:DescriptorSet;
import std;
import NYANData;
import :LogicalDeviceWrapper;
import :DescriptorSetLayout;
import :Object;
import :Buffer;

export namespace nyan::vulkan
{
	struct Descriptor 
	{
		enum class Type {
			StorageBuffer = DescriptorSetLayout::storageBufferBinding,
			UniformBuffer = DescriptorSetLayout::uniformBufferBinding,
			Sampler = DescriptorSetLayout::samplerBinding,
			SampledImage = DescriptorSetLayout::sampledImageBinding,
			StorageImage = DescriptorSetLayout::storageImageBinding,
			InputAttachment = DescriptorSetLayout::inputAttachmentBinding,
			AccelerationStructure = DescriptorSetLayout::accelerationStructureBinding
		};
		uint32_t value;
	};
	struct StorageBufferDescriptor : public Descriptor
	{

	};
	struct UniformBufferDescriptor : public Descriptor
	{

	};
	struct SamplerDescriptor : public Descriptor
	{

	};
	struct SampledImageDescriptor : public Descriptor
	{

	};
	struct StorageImageDescriptor : public Descriptor
	{

	};
	struct InputAttachmentDescriptor : public Descriptor
	{

	};
	struct AccelerationStructureDescriptor : public Descriptor
	{

	};

	class DescriptorSet : public Object<VkDescriptorSet>
	{
	private:
		struct Bitmaps
		{
			DynamicBitset storageBuffers{};
			DynamicBitset uniformBuffers{};
			DynamicBitset samplers{};
			DynamicBitset sampledImages{};
			DynamicBitset storageImages{};
			DynamicBitset inputAttachments{};
			DynamicBitset accelerationStructures{};
		};
		struct Counts
		{
			uint32_t storageBuffers { 0 };
			uint32_t uniformBuffers { 0 };
			uint32_t samplers { 0 };
			uint32_t sampledImages { 0 };
			uint32_t storageImages{ 0 };
			uint32_t inputAttachments{ 0 };
			uint32_t accelerationStructures{ 0 };
		};
	public:
		DescriptorSet(DescriptorSet&) = delete;
		DescriptorSet& operator=(DescriptorSet&) = delete;

		DescriptorSet(DescriptorSet&&) noexcept;
		DescriptorSet& operator=(DescriptorSet&&) noexcept;

		[[nodiscard("must handle potential error")]] std::expected<StorageBufferDescriptor, Error> add(const StorageBuffer& buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) noexcept;

		void update(StorageBufferDescriptor descriptor, const StorageBuffer& buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) noexcept;

		void remove(StorageBufferDescriptor descriptor) noexcept;


		[[nodiscard("must handle potential error")]] static std::expected< DescriptorSet, Error> create(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSet handle, const DescriptorSetLayout& layout) noexcept;
	private:
		DescriptorSet(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSet handle, const DescriptorSetLayout& layout, Bitmaps bitmaps) noexcept;
		const DescriptorSetLayout& r_layout;

		Bitmaps m_bitmaps;
		Counts m_counts;
	};
}
