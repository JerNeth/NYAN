#pragma once


#include <source_location>
#include <string_view>
#include <string>


namespace glfw {

	class Error {
	public:
		explicit Error(int errorCode, std::string_view msg, const std::source_location& location = std::source_location::current()) noexcept;
		[[nodiscard]] std::string_view what() const noexcept;
		[[nodiscard]] int error_code() const noexcept;
	private:
		int m_errorCode {0};
		std::string m_msg{};

	};
}
