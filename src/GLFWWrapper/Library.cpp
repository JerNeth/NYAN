#include "GLFWWrapper/Library.hpp"

#include <cassert>

#include "glfwIncludes.h"

namespace glfw {
	std::expected<Library, Error> Library::create(const CreateInfo& createInfo) noexcept
	{

		{
			int hintValue;
			switch (createInfo.platform)
			{
			default:
			case Platform::AnyPlatform:
				hintValue = GLFW_ANY_PLATFORM;
				break;
			case Platform::Win32:
				hintValue = GLFW_PLATFORM_WIN32;
				break;
			case Platform::Cocoa:
				hintValue = GLFW_PLATFORM_COCOA;
				break;
			case Platform::X11:
				hintValue = GLFW_PLATFORM_X11;
				break;
			case Platform::Wayland:
				hintValue = GLFW_PLATFORM_WAYLAND;
				break;
			case Platform::Null:
				hintValue = GLFW_PLATFORM_NULL;
				break;
			}
#ifdef WIN32
			assert(hintValue == GLFW_PLATFORM_WIN32 ||
				hintValue == GLFW_ANY_PLATFORM ||
				hintValue == GLFW_PLATFORM_NULL);
#endif
			if (hintValue != GLFW_ANY_PLATFORM)
				assert(glfwPlatformSupported(hintValue));
			glfwInitHint(GLFW_PLATFORM, hintValue);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		{
			glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, createInfo.joystickHatButtons ? GLFW_TRUE : GLFW_FALSE);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		{
			int hintValue;
			switch (createInfo.anglePlatformType)
			{
			default:
			case AnglePlatformType::None:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_NONE;
				break;
			case AnglePlatformType::OpenGL:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_OPENGL;
				break;
			case AnglePlatformType::OpenGLES:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_OPENGLES;
				break;
			case AnglePlatformType::D3D9:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_D3D9;
				break;
			case AnglePlatformType::D3D11:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_D3D11;
				break;
			case AnglePlatformType::Vulkan:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_VULKAN;
				break;
			case AnglePlatformType::Metal:
				hintValue = GLFW_ANGLE_PLATFORM_TYPE_METAL;
				break;
			}

			glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, hintValue);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		{
			glfwInitHint(GLFW_COCOA_CHDIR_RESOURCES, createInfo.cocoaChDirResources ? GLFW_TRUE : GLFW_FALSE);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		{
			glfwInitHint(GLFW_COCOA_MENUBAR, createInfo.cocoaMenuBar ? GLFW_TRUE : GLFW_FALSE);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		{
			glfwInitHint(GLFW_X11_XCB_VULKAN_SURFACE, createInfo.X11XCBVulkanSurface ? GLFW_TRUE : GLFW_FALSE);
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		if (glfwInit() != GLFW_TRUE) {
			const char* errorMsg = "";
			if (const int error = glfwGetError(&errorMsg); error != GLFW_NO_ERROR)
				return std::unexpected{ Error{error, errorMsg}};
		}

		return Library(createInfo);
	}

	Library::~Library()
	{
		if (m_initialized)
		{
			glfwTerminate();
		}
	}

	Library::Library(Library&& other) noexcept
	{
		m_initialized = other.m_initialized;
		other.m_initialized = false;
		m_createInfo = other.m_createInfo;
	}

	Library& Library::operator=(Library&& other) noexcept
	{
		if (this != &other)
		{
			m_initialized = other.m_initialized;
			other.m_initialized = false;
			m_createInfo = other.m_createInfo;
		}
		return *this;
	}

	Library::Platform Library::get_platform() const noexcept
	{
		assert(m_initialized);
		switch (glfwGetPlatform())
		{
		case GLFW_PLATFORM_WIN32:
			return Platform::Win32;
		case GLFW_PLATFORM_COCOA:
			return Platform::Cocoa;
		case GLFW_PLATFORM_WAYLAND:
			return Platform::Wayland;
		case GLFW_PLATFORM_X11:
			return Platform::X11;
		case GLFW_PLATFORM_NULL:
		default:
			return Platform::Null;
		}
	}

	Library::Library(const CreateInfo& createInfo) noexcept :
		m_createInfo(createInfo),
		m_initialized(true)
	{
	}

}