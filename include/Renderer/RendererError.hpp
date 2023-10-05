#pragma once

//Current workaround to get source_location with clang 15, TODO: remove when upgrading to 16.0
#ifndef __cpp_consteval
#define __cpp_consteval 201811L  // NOLINT(clang-diagnostic-reserved-macro-identifier)
#endif

#include <source_location>

namespace renderer
{
	class Error
	{
	public:
		enum class Type
		{
			
		};
		explicit Error(Type type, const std::source_location& location = std::source_location::current()) noexcept;
		explicit operator Type() const noexcept
		{
			return m_type;
		}
		[[nodiscard]] Type get_type() const noexcept
		{
			return m_type;
		}
		[[nodiscard]] const char* get_message() const noexcept
		{
			return m_message;
		}

	private:
		Type m_type;
		const char* m_message{ "" };
	};
}