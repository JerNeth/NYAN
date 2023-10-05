#pragma once

//Current workaround to get source_location with clang 15, TODO: remove when upgrading to 16.0
#ifndef __cpp_consteval
#define __cpp_consteval 201811L  // NOLINT(clang-diagnostic-reserved-macro-identifier)
#endif

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