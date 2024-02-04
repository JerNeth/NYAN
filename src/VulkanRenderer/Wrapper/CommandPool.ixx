module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:CommandPool;
import :Error;
import NYANData;

export namespace nyan::vulkan::wrapper
{
	class CommandPool
	{
	public:
		[[nodiscard]] static std::expected<CommandPool, Error> create() noexcept;
	private:
	};

}
