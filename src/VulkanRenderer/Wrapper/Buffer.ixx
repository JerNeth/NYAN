module;

#include <bit>
#include <span>

#include "magic_enum.hpp"
#include "magic_enum_containers.hpp"

#include "volk.h"

export module NYANVulkanWrapper:Buffer;
import NYANData;
import :Allocator;
import :DeletionQueue;
import :Object;
import :Error;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class Buffer : public Object<VkBuffer>
	{
	public:
		enum class UsageFlags : uint32_t
		{
			TransferSrc = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_TRANSFER_SRC_BIT)),
			TransferDst = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_TRANSFER_DST_BIT)),
			UniformTexelBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT)),
			StorageTexelBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT)),
			UniformBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT)),
			StorageBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)),
			IndexBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_INDEX_BUFFER_BIT)),
			VertexBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)),
			IndirectBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT)),
			ShaderDeviceAddress = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)),
			VideoDecodeSrc = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_VIDEO_DECODE_SRC_BIT_KHR)),
			VideoDecodeDst = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_VIDEO_DECODE_DST_BIT_KHR)),
			AccelerationStructureBuildInputReadOnly = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR)),
			AccelerationStructureStorageBit = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR)),
			ShaderBindingTable = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR)),
			VideoEncodeSrc = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_VIDEO_ENCODE_DST_BIT_KHR)),
			VideoEncodeDst = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_VIDEO_ENCODE_SRC_BIT_KHR)),
			SamplerDescriptorBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_SAMPLER_DESCRIPTOR_BUFFER_BIT_EXT)),
			ResourceDescriptorBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT)),
			PushDescriptorDescriptorBuffer = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_PUSH_DESCRIPTORS_DESCRIPTOR_BUFFER_BIT_EXT)),
			MicromapBuildInput = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_MICROMAP_BUILD_INPUT_READ_ONLY_BIT_EXT)),
			MicromapStorage = std::countr_zero(static_cast<uint32_t>(VK_BUFFER_USAGE_MICROMAP_STORAGE_BIT_EXT))
		};
		//using Usage = nyan::util::data::bitset<magic_enum::enum_count<UsageFlags>(), UsageFlags>;
		using Usage = magic_enum::containers::bitset<UsageFlags>;
		enum class Location : uint32_t {
			Device,
			Host
		};
		struct Options {
			std::span<uint32_t> queueFamilies = {}
			bool dedicatedAllocations{ false };
			bool mapable{ false };
		};
	private:
		struct Data 
		{
			Usage usage;
			Location location;
			bool mapable;
			bool shared;
		};

	public:
		Buffer(Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;

		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&& other) noexcept;

		~Buffer() noexcept;

		[[nodiscard]] bool shared() const noexcept;
		[[nodiscard]] bool mapable() const noexcept;

		[[nodiscard]] static std::expected<Buffer, Error> create(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Usage usage) noexcept;

	protected:
		Buffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkBuffer handle, Usage usage) noexcept;

		DeletionQueue& r_deletionQueue;

		Data m_data;
	};

	class StorageBuffer : public Buffer
	{
	public:

		[[nodiscard]] static std::expected<StorageBuffer, Error> create(LogicalDevice& device, size_t size, const Options& options = {}) noexcept;

	private:
		StorageBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkBuffer handle) noexcept;

	};
}
