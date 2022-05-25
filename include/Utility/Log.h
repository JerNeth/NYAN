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
		Logger(Type type = Type::Info) : m_type(type) {

		}
		Logger(const Logger& other) : m_type(other.m_type), m_newLine(other.m_newLine) {

		}
		Logger(Logger&& other) : m_type(other.m_type), m_newLine(other.m_newLine) {
			other.m_newLine = false;
		}
		Logger& operator=(const Logger& other) {
			m_type = other.m_type;
			m_newLine = other.m_newLine;
		}
		Logger& operator=(Logger&& other){
			if (this != &other) {
				m_type = other.m_type;
				m_newLine = other.m_newLine;
				other.m_newLine = false;
			}
		}
		~Logger() {
			if (!m_newLine)
				return;
			stream() << '\n';
		}
		Logger& message(const std::string_view message) {
			stream() << message;
			return *this;
		}
		Logger& location(const std::source_location location = std::source_location::current()) {
			stream() << "file: "
				<< location.file_name() << "("
				<< location.line() << ":"
				<< location.column() << ") `"
				<< location.function_name() << "`: ";
			return *this;
		}
		template<class... Args>
		Logger& format(Args&&... args) {

			stream() << std::format(std::forward<Args>(args)...);
			return *this;
		}
	private:
		std::ostream& stream() {
			if (m_type == Type::Error) {
				return std::cerr;
			}
			if (m_type == Type::Info) {
				return std::cout;
			}
			return std::cout;
		}
		Type m_type;
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