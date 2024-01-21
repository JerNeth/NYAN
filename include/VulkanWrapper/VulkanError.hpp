#pragma once

#include <source_location>
#include <string_view>
#include <string>

#include "VulkanIncludes.h"

namespace vulkan
{
	class Error
	{
	public:
		explicit Error(VkResult result, const std::source_location& location = std::source_location::current()) noexcept;
		explicit operator VkResult() const noexcept;
		[[nodiscard]] VkResult get_result() const noexcept;
		[[nodiscard]] const char* get_message() const noexcept;
	private:
		VkResult m_result {VK_SUCCESS};
		const char* m_message {""};
	};
};