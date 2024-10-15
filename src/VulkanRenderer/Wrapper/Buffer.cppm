module;

//#include <bit>
//#include <expected>
//#include <span>

#include "magic_enum.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkan:Buffer;
import std;
import NYANData;
import :Allocator;
import :DeletionQueue;
import :Object;
import :Error;
import :Queue;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class Buffer : public Object<VkBuffer>
	{
	public:
		enum class UsageFlags : uint8_t
		{
			TransferSrc,
			TransferDst,
			UniformTexelBuffer,
			StorageTexelBuffer,
			UniformBuffer,
			StorageBuffer,
			IndexBuffer,
			VertexBuffer,
			IndirectBuffer,
			ShaderDeviceAddress,
			VideoDecodeSrc,
			VideoDecodeDst,
			AccelerationStructureBuildInputReadOnly,
			AccelerationStructureStorageBit,
			ShaderBindingTable,
			VideoEncodeSrc,
			VideoEncodeDst,
			SamplerDescriptorBuffer,
			ResourceDescriptorBuffer,
			PushDescriptorDescriptorBuffer,
			MicromapBuildInput,
			MicromapStorage,
		};
		using BufferSize = uint32_t;
		using Usage = nyan::bitset<magic_enum::enum_count<UsageFlags>(), UsageFlags>;
		//using Usage = magic_enum::containers::bitset<UsageFlags>;
		struct Options {
			BufferSize size = {};
			Queue::FamilyIndex::Group queueFamilies = {};
			bool dedicatedAllocation { false };
			bool mapable{ false };
		};
	protected:
		struct Data 
		{
			Usage usage;
			Object::Location location;
			Queue::FamilyIndex::Group queueFamilies;
			BufferSize size{ 0 }; //Due to devices not supporting >4GB
			VmaAllocation allocation{ VK_NULL_HANDLE};
			void* ptr{ nullptr };
		};

	public:
		Buffer(Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;

		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&& other) noexcept;


		[[nodiscard]] bool shared() const noexcept;
		[[nodiscard]] void* mapped_data() const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> flush() const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> invalidate() const noexcept;

	protected:
		Buffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;
		~Buffer() noexcept;

		DeletionQueue& r_deletionQueue;
		Allocator& r_allocator;
		Data m_data;
	};

	class StorageBuffer : public Buffer
	{
	public:

		[[nodiscard("must handle potential error")]] static std::expected<StorageBuffer, Error> create(LogicalDevice& device, const Options& options = {}) noexcept;

	private:
		StorageBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;

	};

	class UniformBuffer : public Buffer
	{
	public:

		[[nodiscard("must handle potential error")]] static std::expected<UniformBuffer, Error> create(LogicalDevice& device, const Options& options = {}) noexcept;

	private:
		UniformBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;

	};

	class AccelerationStructureBuffer : public Buffer
	{
	public:

		[[nodiscard("must handle potential error")]] static std::expected<AccelerationStructureBuffer, Error> create(LogicalDevice& device, const Options& options = {}) noexcept;

	private:
		AccelerationStructureBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;

	};

	class StagingBuffer : public Buffer
	{
	public:

	private:
		StagingBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;
	};
	

	class MeshBuffer : public Buffer
	{
	public:

	private:
		MeshBuffer(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, Allocator& allocator, VkBuffer handle, Data data) noexcept;
	};
}
