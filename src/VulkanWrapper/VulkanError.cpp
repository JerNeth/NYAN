#include "VulkanWrapper/VulkanError.hpp"

namespace vulkan {
	Error::Error(const VkResult result, [[maybe_unused]] const std::source_location& location) noexcept :
		m_result(result),
		m_message("")
	{
	}

	Error::operator VkResult() const noexcept
	{
		return m_result;
	}

	VkResult Error::get_result() const noexcept
	{
		return m_result;
	}

	const char* Error::get_message() const noexcept
	{
		return m_message;
	}
}