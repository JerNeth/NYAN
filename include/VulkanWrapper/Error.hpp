#pragma once

#ifndef __cpp_consteval
#define __cpp_consteval 201811L
#endif
#include <source_location>
#include <string_view>
#include <string>

namespace vulkan
{
	class Error
	{
	public:
		explicit Error(VkResult result, const std::source_location location = std::source_location::current()) noexcept;
	private:
	};
};