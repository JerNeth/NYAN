#ifndef UTEXCEPTIONS_H
#define UTEXCEPTIONS_H
#pragma once
#include <source_location>
#include <exception>
namespace Utility {
	class FeatureNotSupportedException : public std::exception {
	public:
		FeatureNotSupportedException(const std::source_location location = std::source_location::current()) {}
		FeatureNotSupportedException(const char* msg) : m_msg(msg) {}
		const char* what() const override {
			if (m_msg)
				return m_msg;
			else 
				return "Feature not supported";
			
		}
	private:
		const char* m_msg{ nullptr };
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
}

#endif !UTEXCEPTIONS_H