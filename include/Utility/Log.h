#ifndef UTILITYLOG_H
#define UTILITYLOG_H
#pragma once

#include <string_view>
#ifndef __cpp_consteval
#define __cpp_consteval 201811L
#endif
#include <source_location>
#include <format>
#include <iostream>


namespace Utility {
	class Logger {
	public:
		enum class Type {
			Verbose,
			Info,
			Warn,
			Error
		};
		constexpr Logger(Type type = Type::Info) : m_type(type) {

		}
		constexpr Logger(const Logger& other) : m_type(other.m_type), m_newLine(other.m_newLine) {

		}
		constexpr Logger(Logger&& other) noexcept : m_type(other.m_type), m_newLine(other.m_newLine) {
			other.m_newLine = false;
		}
		constexpr Logger& operator=(const Logger& other) {
			m_type = other.m_type;
			m_newLine = other.m_newLine;
			return *this;
		}
		constexpr Logger& operator=(Logger&& other) noexcept {
			if (this != &other) {
				m_type = other.m_type;
				m_newLine = other.m_newLine;
				other.m_newLine = false;
			}
			return *this;
		}
		~Logger() {
			if (!m_newLine || !loggingEnabled)
				return;
			stream() << '\n';
		}
		const Logger& message(const std::string_view message) const& {
			if constexpr (loggingEnabled)
				stream() << message;
			return *this;
		}
		Logger&& message(const std::string_view message) && {
			if constexpr (loggingEnabled)
				stream() << message;
			return std::move(*this);
		}
		const Logger& location(const std::source_location location = std::source_location::current()) const & {
			if constexpr (loggingEnabled)
				stream() << "file: "
				<< location.file_name() << "("
				<< location.line() << ":"
				<< location.column() << ") `"
				<< location.function_name() << "`: ";
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
		}
		Logger&& location(const std::source_location location = std::source_location::current()) && {
			if constexpr (loggingEnabled)
				stream() << "file: "
				<< location.file_name() << "("
				<< location.line() << ":"
				<< location.column() << ") `"
				<< location.function_name() << "`: ";
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return std::move(*this);
		}
		template<typename ...Args>
		const Logger& format(std::string_view view, Args&&... args) const & {
			if constexpr (loggingEnabled)
				stream() << std::vformat(view, std::make_format_args(args...));
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
		}
		template<typename ...Args>
		Logger&& format(std::string_view view, Args&&... args) && {
			if constexpr (loggingEnabled)
				stream() << std::vformat(view, std::make_format_args(args...));
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return std::move(*this);
		}
	private:
		std::ostream& stream() const {
			if (m_type == Type::Error) {
				return std::cerr;
			}
			if (m_type == Type::Info) {
				return std::cout;
			}
			return std::cout;
		}
		Type m_type {Type::Info};
		bool m_newLine = true;
		static constexpr bool loggingEnabled = true;
		
	};
	inline Logger log()
	{
		return Logger{};
	}
	inline Logger log_verbose()
	{
		return Logger{ Logger::Type::Verbose };
	}
	inline Logger log_warning()
	{
		return Logger{ Logger::Type::Warn };
	}
	inline Logger log_info()
	{
		return Logger{ Logger::Type::Info };
	}
	inline Logger log_error()
	{
		return Logger{ Logger::Type::Error };
	}
	inline Logger log(const std::string_view message)
	{
		return log().message(message);
	}
	inline Logger log_verbose(const std::string_view message)
	{
		return log_verbose().message(message);
	}
	inline Logger log_warning(const std::string_view message)
	{
		return log_warning().message(message);
	}
	inline Logger log_info(const std::string_view message)
	{
		return log_info().message(message);
	}
	inline Logger log_error(const std::string_view message)
	{
		return log_error().message(message);
	}
}
#endif //!UTILITYLOG_H