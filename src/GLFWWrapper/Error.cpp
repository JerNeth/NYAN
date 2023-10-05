#include "GLFWWrapper/Error.hpp"

#include <format>
#include <cassert>

#include "glfwIncludes.h"

namespace glfw {
	
	Error::Error(const int errorCode, std::string_view msg, const std::source_location& location) noexcept :
		m_errorCode(errorCode)
	{
		switch(m_errorCode)
		{
		case GLFW_NOT_INITIALIZED:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): library not initialized. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_NO_CURRENT_CONTEXT:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): no current OpenGL/OpenGLES context. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_INVALID_ENUM:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): invalid enum value. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_INVALID_VALUE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): invalid value. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_OUT_OF_MEMORY:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): out of memory. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_API_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): api unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_VERSION_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): OpenGL/OpenGLES version unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_PLATFORM_ERROR:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): platform error. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_FORMAT_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): format unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_NO_WINDOW_CONTEXT:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): no window context. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_CURSOR_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): cursor unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_FEATURE_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): feature unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_FEATURE_UNIMPLEMENTED:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): feature unimplemented. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_PLATFORM_UNAVAILABLE:
			m_msg = std::format("{}({}): {}: GLFW error ({:#x}): platform unavailable. {}", location.file_name(), location.line(), location.function_name(), errorCode, msg);
			break;
		case GLFW_NO_ERROR:
		default:
			assert(false);
			break;
		}
	}
	
	std::string_view Error::what() const noexcept
	{
		return m_msg;
	}
	
	int Error::error_code() const noexcept
	{
		return m_errorCode;
	}

}