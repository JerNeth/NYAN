#pragma once

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