module;

#include <atomic>
#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:Semaphore;
import :Error;
import :DeletionQueue;
import :Object;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class Semaphore : public Object<VkSemaphore>
	{
	public:
	protected:
		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;

		Semaphore(Semaphore&& other) noexcept;

		Semaphore& operator=(Semaphore&& other) noexcept;
		~Semaphore() noexcept;

		Semaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept;

		DeletionQueue& r_deletionQueue;
	};

	class TimelineSemaphore : public Semaphore
	{
	public:
		TimelineSemaphore(const TimelineSemaphore&) = delete;
		TimelineSemaphore(TimelineSemaphore&& other) noexcept;
		
		TimelineSemaphore& operator=(const TimelineSemaphore&) = delete;
		TimelineSemaphore& operator=(TimelineSemaphore&& other) noexcept;

		[[nodiscard("must handle potential error")]] std::expected<uint64_t, Error> get_value() const noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<TimelineSemaphore, Error> create(LogicalDevice& device, uint64_t initialValue = 0) noexcept;
	private:
		TimelineSemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t initialValue) noexcept;

		std::atomic<uint64_t> m_timeline;
	};

	class BinarySemaphore : public Semaphore
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<BinarySemaphore, Error> create(LogicalDevice& device) noexcept;
	private:
		BinarySemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept;

	};
}
