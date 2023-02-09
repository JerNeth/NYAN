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
			if (!m_newLine)
				return;
			stream() << '\n';
		}
		const Logger& message(const std::string_view message) const& {
			stream() << message;
			return *this;
		}
		const Logger&& message(const std::string_view message) const && {
			stream() << message;
			return std::move(*this);
		}
		const Logger& location(const std::source_location location = std::source_location::current()) const & {
			stream() << "file: "
				<< location.file_name() << "("
				<< location.line() << ":"
				<< location.column() << ") `"
				<< location.function_name() << "`: ";
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
		}
		const Logger& location(const std::source_location location = std::source_location::current()) const && {
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
			stream() << std::vformat(view, std::make_format_args(args...));
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
		}
		template<typename ...Args>
		const Logger&& format(std::string_view view, Args&&... args) const && {
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
	};
	inline Logger log()
	{
		return Logger{};
	}
	inline Logger log_warning()
	{
		return Logger{ Logger::Type::Warn };
	}
	inline Logger log_error()
	{
		return Logger{ Logger::Type::Error };
	}
	inline Logger log(const std::string_view message)
	{
		return Logger().message(message);
	}
	inline Logger log_warning(const std::string_view message)
	{
		return Logger{ Logger::Type::Warn }.message(message);
	}
	inline Logger log_error(const std::string_view message)
	{
		return Logger{ Logger::Type::Error }.message(message);
	}
}
#endif !UTILITYLOG_H