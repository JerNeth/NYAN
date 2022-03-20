#ifndef UTILITYLOG_H
#define UTILITYLOG_H
#pragma once

#include <string_view>
#include <source_location>

namespace Utility {
	inline void log(const std::string_view message, const std::source_location location = std::source_location::current())
	{
		std::cout << "file: "
			<< location.file_name() << "("
			<< location.line() << ":"
			<< location.column() << ") `"
			<< location.function_name() << "`: "
			<< message << '\n';
	}
	inline void log_error(const std::string_view message, const std::source_location location = std::source_location::current())
	{
		std::cout << "file: "
			<< location.file_name() << "("
			<< location.line() << ":"
			<< location.column() << ") `"
			<< location.function_name() << "`: "
			<< message << '\n';
	}
}
#endif !UTILITYLOG_H