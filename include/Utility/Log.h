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
			Error
		};
		constexpr Logger(Type type = Type::Info) : m_type(type) {

		}
		constexpr Logger(const Logger& other) : m_type(other.m_type), m_newLine(other.m_newLine) {

		}
		constexpr Logger(Logger&& other) : m_type(other.m_type), m_newLine(other.m_newLine) {
			other.m_newLine = false;
		}
		Logger& operator=(const Logger& other) {
			m_type = other.m_type;
			m_newLine = other.m_newLine;
			return *this;
		}
		Logger& operator=(Logger&& other){
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
		const Logger& message(const std::string_view message) const {
			stream() << message;
			return *this;
		}
		//Logger& location() {
		const Logger& location(const std::source_location location = std::source_location::current()) const {
			stream() << "file: "
				<< location.file_name() << "("
				<< location.line() << ":"
				<< location.column() << ") `"
				<< location.function_name() << "`: ";
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
		}
		template<typename ...Args>
		const Logger& format(std::string_view view, Args&&... args) const {
			stream() << std::vformat(view, std::make_format_args(args...));
			//OutputDebugString(std::vformat(view, std::make_format_args(args...)));
			return *this;
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
	inline Logger log_error()
	{
		return Logger{ Logger::Type::Error };
	}
	inline Logger log(const std::string_view message)
	{
		return Logger().message(message);
	}
	inline Logger log_error(const std::string_view message)
	{
		return Logger(Logger::Type::Error).message(message);
	}
}
#endif !UTILITYLOG_H