module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:CommandPool;
import :LogicalDeviceWrapper;
import :Error;
import :Object;
import :DeletionQueue;
import :CommandBuffer;
import :Queue;
import NYANData;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class CommandPool : Object<VkCommandPool>
	{
	public:
		CommandPool(CommandPool&& other) noexcept;
		CommandPool& operator=(CommandPool&& other) noexcept;
		CommandPool(CommandPool& other) = delete;
		CommandPool& operator=(CommandPool& other) = delete;

		~CommandPool() noexcept;

		[[nodiscard("must handle potential error")]] std::expected<void, Error> reset(bool release = false) noexcept;

		[[nodiscard("must handle potential error")]] std::expected<CommandBuffer, Error> allocate_command_buffer() noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<CommandPool, Error> create(LogicalDevice& device, Queue& queue, bool transient = true, bool reset = false) noexcept;
	private:
		CommandPool(const LogicalDeviceWrapper& device, VkCommandPool handle, DeletionQueue& deletionQueue, Queue& queue) noexcept;

		DeletionQueue& r_deletionQueue;
		Queue& r_queue;
	};

}
