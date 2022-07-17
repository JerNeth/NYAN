#ifndef UTEXCEPTIONS_H
#define UTEXCEPTIONS_H
#pragma once
#include <source_location>
#include <filesystem>
#include <format>
#include <exception>
namespace Utility {
	class FeatureNotSupportedException : public std::exception {
	public:
		FeatureNotSupportedException(const std::source_location location = std::source_location::current())
			: m_msg(std::format("{}: {} [{}]\n: Feature not supported by Device", location.file_name(), location.function_name(), location.line())) 
		{
		}
		const char* what() const override {
			return m_msg.c_str();
			
		}
	private:
		std::string m_msg{};
	};
	class DeviceLostException : public std::exception {
	public:
		DeviceLostException(const std::source_location location = std::source_location::current())
		{

		}
		DeviceLostException(const char* msg) : m_msg(msg) {}
		const char* what() const override {
			if (m_msg)
				return m_msg;
			else
				return "Device Lost";

		}
	private:
		const char* m_msg{ nullptr };
	};
	class FileNotFoundException : public std::exception {
	public:
		FileNotFoundException(const std::filesystem::path& file, const std::source_location location = std::source_location::current()) 
			: m_msg(std::format("{}({}): {}: File does not exist: \"{}\"", location.file_name(), location.line(), location.function_name(), file.string()))
		{
		}
		const char* what() const override {
			return m_msg.c_str();

		}
	private:
		std::string m_msg{};
	};
	class FileTypeNotSupportedException : public std::exception {
	public:
		FileTypeNotSupportedException(const std::source_location location = std::source_location::current())
		{

		}
		FileTypeNotSupportedException(const char* msg) : m_msg(msg) {}
		FileTypeNotSupportedException(const std::string& msg) : m_msg(msg) {}
		const char* what() const override {
			if (!m_msg.empty())
				return m_msg.c_str();
			else
				return "File type not supported";

		}
	private:
		std::string m_msg{};
	};
}

#endif !UTEXCEPTIONS_H