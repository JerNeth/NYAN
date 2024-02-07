module;

#include <expected>

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkanWrapper:Allocator;
import :LogicalDeviceWrapper;
import :Error;
import :Object;

export namespace nyan::vulkan::wrapper
{
	class Instance;
	class PhysicalDevice;

	class Allocation {
	public:

	private:

	};

	class Allocator : public Object<VmaAllocator>
	{
	public:
		Allocator(Allocator& other) = delete;
		Allocator& operator=(Allocator& other) = delete;

		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(Allocator&& other) noexcept;

		~Allocator();

		[[nodiscard]] std::expected<void*, Error> map_memory(const VmaAllocation allocation) const noexcept;
		void unmap_memory(const VmaAllocation allocation) const noexcept;
		[[nodiscard]] std::expected<void*, Error> flush(const VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const noexcept;
		[[nodiscard]] std::expected<void*, Error> invalidate(const VmaAllocation allocation, VkDeviceSize offset, VkDeviceSize size) const noexcept;

		void free_allocation(VmaAllocation allocation) const noexcept;

		[[nodiscard]] static std::expected<Allocator, Error> create(const Instance& instance, const LogicalDeviceWrapper& logicalDevice,
		                                                            const PhysicalDevice& physicalDevice, VmaAllocatorCreateFlags createFlags) noexcept;
	private:
		Allocator(const LogicalDeviceWrapper& device, VmaAllocator allocator) noexcept;
	};
}
