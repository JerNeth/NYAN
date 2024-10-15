module;

//#include <atomic>
//#include <expected>

#include "volk.h"

export module NYANVulkan:Semaphore;
import std;
import :Error;
import :DeletionQueue;
import :Object;

export namespace nyan::vulkan
{
	class Queue;
	class Swapchain;
	class LogicalDevice;
	class Semaphore : public Object<VkSemaphore>
	{
	public:
		friend class Queue;
		friend class Swapchain;
	protected:
		Semaphore(const Semaphore&) = delete;
		Semaphore& operator=(const Semaphore&) = delete;

		Semaphore(Semaphore&& other) noexcept;

		Semaphore& operator=(Semaphore&& other) noexcept;
		~Semaphore() noexcept;

		Semaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t timeline) noexcept;


		DeletionQueue& r_deletionQueue;

		std::atomic<uint64_t> m_timeline;

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

		friend void swap(TimelineSemaphore& a, TimelineSemaphore& b) noexcept;
	private:
		TimelineSemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle, uint64_t initialValue) noexcept;

	};

	class BinarySemaphore : public Semaphore
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<BinarySemaphore, Error> create(LogicalDevice& device) noexcept;

		[[nodiscard]] bool signaled() const noexcept;
		[[nodiscard]] bool waited() const noexcept;


		friend void swap(BinarySemaphore& a, BinarySemaphore& b) noexcept;
	private:
		BinarySemaphore(const LogicalDeviceWrapper& device, DeletionQueue& deletionQueue, VkSemaphore handle) noexcept;

	};
	void swap(TimelineSemaphore& a, TimelineSemaphore& b) noexcept
	{
		if (std::addressof(a) != std::addressof(b)) {
			std::swap(a.ptr_device, b.ptr_device);
			std::swap(a.m_handle, b.m_handle);
			a.m_timeline = b.m_timeline.exchange(a.m_timeline.load());
		}
	}
	void swap(BinarySemaphore& a, BinarySemaphore& b) noexcept
	{
		if (std::addressof(a) != std::addressof(b)) {
			std::swap(a.ptr_device, b.ptr_device);
			std::swap(a.m_handle, b.m_handle);
			a.m_timeline = b.m_timeline.exchange(a.m_timeline.load());
		}
	}
}
