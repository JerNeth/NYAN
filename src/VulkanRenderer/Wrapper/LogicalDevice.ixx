module;

#include <expected>
#include <array>

export module NYANVulkanWrapper:LogicalDevice;
import :Allocator;
import :DeletionQueue;
import :Error;
import :Instance;
import :LogicalDeviceWrapper;
import :PhysicalDevice;
import :Queue;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice
	{
	public:
		LogicalDevice(LogicalDevice&) = delete;
		LogicalDevice(LogicalDevice&&) noexcept;

		LogicalDevice& operator=(LogicalDevice&) = delete;
		LogicalDevice& operator=(LogicalDevice&&) noexcept;

		~LogicalDevice() noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<LogicalDevice, LogicalDeviceCreationError> create(Instance& instance, PhysicalDevice physicalDevice,
			const VkAllocationCallbacks* allocatorCallbacks, PhysicalDevice::Extensions enabledExtensions,
			const QueueContainer<float>& queuePriorities) noexcept;

		[[nodiscard]] const PhysicalDevice& get_physical_device() const noexcept;

		[[nodiscard]] const LogicalDeviceWrapper& get_device() const noexcept;

		[[nodiscard]] DeletionQueue& get_deletion_queue() noexcept;

		[[nodiscard]] const PhysicalDevice::Extensions& get_enabled_extensions() const noexcept;

		[[nodiscard]] const std::vector<Queue>& get_queues(Queue::Type type) const noexcept;

		[[nodiscard]] std::vector<Queue>& get_queues(Queue::Type type) noexcept;

		[[nodiscard]] Allocator& get_allocator() noexcept;
		[[nodiscard]] const Allocator& get_allocator() const noexcept;

	private:

		LogicalDevice(LogicalDeviceWrapper device, PhysicalDevice physicalDevice, Allocator allocator, PhysicalDevice::Extensions enabledExtensions,
			const QueueContainer<float>& queuePriorities) noexcept;

		void init_queues(const QueueContainer<float>& queuePriorities) noexcept;

		LogicalDeviceWrapper m_deviceWrapper;
		PhysicalDevice m_physicalDevice;
		Allocator m_allocator;
		DeletionQueue m_deletionQueue;

		QueueContainer<Queue> m_queues;


		PhysicalDevice::Extensions m_enabledExtensions;

	};
}
