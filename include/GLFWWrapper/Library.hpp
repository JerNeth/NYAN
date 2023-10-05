#pragma once
#include "Error.hpp"

#include <expected>

namespace glfw
{
	class Library
	{
	public:
		enum class Platform : uint32_t
		{
			AnyPlatform,
			Win32,
			Cocoa,
			X11,
			Wayland,
			Null
		};
		enum class AnglePlatformType : uint32_t
		{
			None,
			OpenGL,
			OpenGLES,
			D3D9,
			D3D11,
			Vulkan,
			Metal
		};

		struct CreateInfo
		{
			Platform platform { Platform::AnyPlatform };
			bool joystickHatButtons { true };
			AnglePlatformType anglePlatformType {AnglePlatformType::None};
			bool cocoaChDirResources { true };
			bool cocoaMenuBar { true };
			//if true prefers VK_KHR_xcb_surface, VK_KHR_xlib_surface otherwise
			bool X11XCBVulkanSurface{ true };

		};
		static std::expected<Library, Error> create(const CreateInfo& createInfo = CreateInfo{ Platform::AnyPlatform, true,  AnglePlatformType::None, true, true, true}) noexcept;
		~Library();
		Library(Library&) = delete;
		Library(Library&& other) noexcept;
		Library& operator=(Library&) = delete;
		Library& operator=(Library&& other) noexcept;
		Platform get_platform() const noexcept;
	private:
		explicit Library(const CreateInfo& createInfo) noexcept;

		CreateInfo m_createInfo {};
		bool m_initialized {false};
	};
}