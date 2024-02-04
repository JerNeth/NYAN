module;

#include "volk.h"

export module NYANVulkanWrapper:Buffer;
import :Object;
import :Error;

export namespace nyan::vulkan::wrapper
{
	class Buffer : public Object<VkBuffer>
	{
	public:
		Buffer(Buffer&) = delete;
		Buffer(Buffer&& other) noexcept;

		Buffer& operator=(Buffer&) = delete;
		Buffer& operator=(Buffer&& other) noexcept;

		~Buffer() noexcept;

		[[nodiscard]] static std::expected<Buffer, Error> create(const LogicalDeviceWrapper& device) noexcept;
	private:
		Buffer(const LogicalDeviceWrapper& device, VkBuffer handle) noexcept;
	};
}
