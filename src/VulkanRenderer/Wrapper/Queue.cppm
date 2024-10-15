module;

//#include <array>
//#include <bit>
//#include <expected>
//#include <span>
//#include <string_view>
//#include <vector>
//#include <memory>

#include "magic_enum.hpp"
#include "magic_enum_containers.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkan:Queue;
import std;

import NYANData;

import :Error;
import :CommandPool;
import :LogicalDeviceWrapper;
import :Object;
import :Semaphore;

export namespace nyan::vulkan
{
	class CommandBuffer;
	class Fence;
	class Swapchain;
	class Queue : public Object<VkQueue>
	{
	public:

		//Specification (1.3.279) says: If an implementation exposes any queue family that supports graphics operations, at least one queue family of at least one physical device exposed by the implementation must support both graphics and compute operations.
		//Meaning, it is reasonable to assume, albeit not technically correct, that each graphics queue has also compute capabilities.
		//Specification (1.3.279) says: All commands that are allowed on a queue that supports transfer operations are also allowed on a queue that supports either graphics or compute operations. 
		//Thus, if the capabilities of a queue family include VK_QUEUE_GRAPHICS_BIT or VK_QUEUE_COMPUTE_BIT, then reporting the VK_QUEUE_TRANSFER_BIT capability separately for that queue family is optional.
		//Meaning, all graphics and compute queues also implicitly support transfer
		//Furthermore, transfer here means exclusively transfer.
		enum class Type : uint32_t
		{
			Graphics = 0,
			Compute = 1,
			Transfer = 2,
			Encode = 3,
			Decode = 4
		};
		struct FamilyIndex 
		{
			uint32_t value;
			constexpr FamilyIndex() noexcept : value(0) {};
			constexpr FamilyIndex(uint32_t _value) noexcept : value(_value) {};

			static constexpr size_t max = 16;
			struct Group
			{
				uint16_t bitmask; //AFAIK: Devices with more than 16 queue families do not exist
				static_assert(sizeof(bitmask) * 8 == max);
				constexpr void set(FamilyIndex idx) noexcept {
					assert(idx.value < max);
					bitmask |= 1u << idx.value;
				}
				constexpr bool test(FamilyIndex idx) const noexcept {
					assert(idx.value < max);
					return bitmask & (1u << idx.value);
				}
				constexpr bool empty() const noexcept {
					return bitmask == 0;
				}
				constexpr uint32_t size() const noexcept {
					return std::popcount(bitmask);
				}
				constexpr void fill(std::span<uint32_t> dst) const noexcept {
					assert(dst.size() <= size());
					auto bitmaskCopy = bitmask;
					size_t count = 0;
					while (bitmaskCopy && count < dst.size()) {
						uint32_t idx = std::countr_zero(bitmaskCopy);
						bitmaskCopy ^= 1ull << idx;
						dst[count++] = idx;
					}
				}
			};
			static constexpr decltype(value) invalid = ~0u;
			static constexpr decltype(value) ignored = VK_QUEUE_FAMILY_IGNORED;
			static constexpr decltype(value) external = VK_QUEUE_FAMILY_EXTERNAL;
		};
		static constexpr size_t maxNumCommandPools = 1; //Maximum amount of Command Pools per Queue;
	public:

		Queue(Queue&) = delete;
		Queue(Queue&& other) noexcept;

		Queue& operator=(Queue&) = delete;
		Queue& operator=(Queue&& other) noexcept;
		Queue(const LogicalDeviceWrapper& device, VkQueue handle, Type type, FamilyIndex queueFamilyIndex, float priority, std::array<uint32_t, 3> transferGranularity) noexcept;

		[[nodiscard("must handle potential error")]] std::expected<void, Error> wait_idle() const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit(CommandBuffer& cmd, BinarySemaphore& waitSemaphore, BinarySemaphore& signalSemaphore) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit(CommandBuffer& cmd) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit_signal(CommandBuffer& cmd, BinarySemaphore& signalSemaphore) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit_wait(CommandBuffer& cmd, BinarySemaphore& waitSemaphore) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit(BinarySemaphore& semaphore) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit2(CommandBuffer& cmd, Fence* fence = nullptr) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<uint64_t, Error> submit2(std::span<CommandBuffer> cmds, TimelineSemaphore& signal, TimelineSemaphore& wait, uint64_t value) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> submit2(std::span<CommandBuffer> cmds, TimelineSemaphore& semaphore, uint64_t value) const noexcept;

		[[nodiscard("must handle potential error")]] std::expected<void, Error> present(Swapchain& swapchain, BinarySemaphore* semaphore = nullptr) const noexcept;

		void insert_debug_utils_label(const char* labelName, std::array<float, 4> color) const noexcept;
		void begin_debug_utils_label(const char* labelName, std::array<float, 4> color) const noexcept;
		void end_debug_utils_label() const noexcept;

		[[nodiscard]] Type get_type() const noexcept;
		[[nodiscard]] FamilyIndex get_queue_family_index() const noexcept;
		[[nodiscard]] float get_priority() const noexcept;
		[[nodiscard]] bool is_present_capable() const noexcept;
		[[nodiscard]] bool is_sparse_capable() const noexcept;

		[[nodiscard]] const std::array<uint32_t, 3>& transfer_granularity() const noexcept;

		//[[nodiscard]] auto get_command_pools() const noexcept {
		//	return m_commandPools;
		//}
		//[[nodiscard]] auto get_command_pools() noexcept {
		//	return m_commandPools;
		//}
		
	private:

		Type m_type;
		FamilyIndex m_queueFamilyIndex {FamilyIndex::invalid };
		float m_priority {0.f};
		bool m_presentCapable {false};
		bool m_sparseCapable{ false };
		bool m_enableLowLatencyNV{ false };
		std::array<uint32_t, 3> m_minImageTransferGranularity{ 0, 0, 0 };
		uint64_t m_presentId{ 0 };
		//nyan::StaticVector< CommandPool, maxNumCommandPools> m_commandPools;
	};
	template<typename T>
	using QueueTypeArray = magic_enum::containers::array<Queue::Type,  T>;

	constexpr size_t maxQueueCount = 4; //Maximum of 4 queues per family (currently more do not make sense)
	template<typename T>
	using QueueContainer = QueueTypeArray<nyan::StaticVector<T, maxQueueCount>>;

}
