module;

//#include <array>
//#include <expected>
//#include <span>

#include <volk.h>
#include <vk_mem_alloc.h>

export module NYANVulkan:LogicalDevice;
import std;
import :Allocator;
import :DeletionQueue;
import :Error;
import :Instance;
import :LogicalDeviceWrapper;
import :PhysicalDevice;
import :Queue;

export namespace nyan::vulkan
{
	class LogicalDevice
	{
	public:
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) noexcept; // moving invalidates all references to device wrapper

		LogicalDevice& operator=(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&&) noexcept; // moving invalidates all references to device wrapper

		~LogicalDevice() noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<LogicalDevice, LogicalDeviceCreationError> create(Instance& instance, PhysicalDevice physicalDevice,
			const VkAllocationCallbacks* allocatorCallbacks, PhysicalDevice::Extensions enabledExtensions,
			const QueueContainer<float>& queuePriorities) noexcept;

		[[nodiscard]] const PhysicalDevice& get_physical_device() const noexcept;

		[[nodiscard]] const LogicalDeviceWrapper& get_device() const noexcept;

		[[nodiscard]] DeletionQueue& get_deletion_queue() noexcept;

		[[nodiscard]] const PhysicalDevice::Extensions& get_enabled_extensions() const noexcept;

		[[nodiscard]] std::span<const Queue> get_queues(Queue::Type type) const noexcept;

		[[nodiscard]] std::span<Queue> get_queues(Queue::Type type) noexcept;

		[[nodiscard]] Allocator& get_allocator() noexcept;
		[[nodiscard]] const Allocator& get_allocator() const noexcept;

		[[nodiscard]] std::expected<void, Error> wait_idle() const noexcept;

	private:

		LogicalDevice(LogicalDeviceWrapper device, PhysicalDevice physicalDevice, VmaAllocator allocator, PhysicalDevice::Extensions enabledExtensions,
			const QueueContainer<float>& queuePriorities) noexcept;

		void init_queues(const QueueContainer<float>& queuePriorities) noexcept;

		void update_wrapper_pointers() noexcept;

		LogicalDeviceWrapper m_deviceWrapper;
		PhysicalDevice m_physicalDevice;
		Allocator m_allocator;
		DeletionQueue m_deletionQueue;

		QueueContainer<Queue> m_queues;


		PhysicalDevice::Extensions m_enabledExtensions;

	};
}
