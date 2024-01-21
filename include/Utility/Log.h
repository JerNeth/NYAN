#ifndef UTILITYLOG_H
#define UTILITYLOG_H
#pragma once

#include <string_view>
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
	private:

		explicit constexpr Logger(Type type = Type::Info) noexcept : m_type(type) {

		}

	public:

		constexpr Logger(const Logger& other) noexcept = default;

		constexpr Logger(Logger&& other) noexcept : m_type(other.m_type), m_newLine(other.m_newLine) {
			other.m_newLine = false;
		}
		constexpr Logger& operator=(const Logger& other) noexcept {
			if (this != std::addressof(other)) {
				m_type = other.m_type;
				m_newLine = other.m_newLine;
			}
			return *this;
		}
		constexpr Logger& operator=(Logger&& other) noexcept {
			if (this != std::addressof(other)) {
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
		Logger&& message(const std::string_view message) &&  {
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
		[[nodiscard]] std::ostream& stream() const noexcept{
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

	public:
		static Logger verbose() noexcept
		{
			return Logger{ Logger::Type::Verbose };
		}
		static Logger warning() noexcept
		{
			return Logger{ Logger::Type::Warn };
		}
		static Logger info() noexcept
		{
			return Logger{ Logger::Type::Info };
		}
		static Logger error() noexcept
		{
			return Logger{ Logger::Type::Error };
		}
		static Logger verbose_message(const std::string_view message)
		{
			return verbose().message(message);
		}
		static Logger warning_message(const std::string_view message)
		{
			return warning().message(message);
		}
		static Logger info_message(const std::string_view message)
		{
			return info().message(message);
		}
		static Logger error_message(const std::string_view message)
		{
			return error().message(message);
		}
	};
}
#endif //!UTILITYLOG_H