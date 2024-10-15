module;

//#include <chrono>
//#include <limits>
//#include <expected>

#include "volk.h"

export module NYANVulkan:Fence;
import std;
import :Error;
import :LogicalDeviceWrapper;
import :Object;

export namespace nyan::vulkan
{
	class LogicalDevice;
	class Fence : public Object<VkFence>
	{
	public:
		Fence(Fence&& other) noexcept;
		Fence& operator=(Fence&& other) noexcept;

		Fence(Fence& other) noexcept = delete;
		Fence& operator=(Fence& other) noexcept = delete;

		~Fence() noexcept;

		[[nodiscard]] bool is_signaled() const noexcept;
		void set_signaled() noexcept;

		[[nodiscard("must handle potential error")]] std::expected<bool, Error> wait(std::chrono::duration<uint64_t,std::nano> timeout = std::chrono::duration<uint64_t, std::nano>{std::numeric_limits<uint64_t>::max()}) const noexcept;
		[[nodiscard("must handle potential error")]] std::expected<void, Error> reset() noexcept;
		[[nodiscard("must handle potential error")]] std::expected<bool, Error> get_status() const noexcept;

		[[nodiscard("must handle potential error")]] static std::expected<Fence, Error> create(const LogicalDeviceWrapper& device) noexcept;
	private:
		Fence(const LogicalDeviceWrapper& device, VkFence handle) noexcept;

		bool m_signaled{ false };
	};
}
