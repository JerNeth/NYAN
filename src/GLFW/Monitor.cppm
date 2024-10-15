module;

//#include <algorithm>
//#include <cassert>
//#include <expected>
//#include <string>
//#include <string_view>
//#include <span>
//#include <vector>

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN  
#define GLFW_EXPOSE_NATIVE_WIN32
//#include <windows.h>
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>


export module NYANGLFW:Monitor; 
import std;
import NYANAssert;
import NYANLog;
import :Error;
import :ModuleSettings;



#ifdef WIN32
static std::string convert_wchar_to_utf8(wchar_t* wstring) noexcept {
	auto res = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, nullptr, 0, nullptr, nullptr);
	std::string temp (res, '\0');
	res = WideCharToMultiByte(CP_UTF8, 0, wstring, -1, temp.data(), temp.size(), nullptr, nullptr);

	return temp;
}
#endif

export namespace nyan::glfw
{

	class Monitor
	{
	public:
		friend class Library;
		friend class Window;

		struct VideoMode {
			uint32_t width : 16;
			uint32_t height : 16;
			uint32_t redBits : 8;
			uint32_t greenBits : 8;
			uint32_t blueBits : 8 ;
			uint32_t refreshRate : 16;
			[[nodiscard]] bool operator==(const VideoMode& rhs) const noexcept = default;
		};

	public:
		[[nodiscard]] std::expected<VideoMode, nyan::glfw::Error> get_current_video_mode() noexcept;

		[[nodiscard]] std::span<const VideoMode> get_video_modes() const noexcept {
			return { m_videoModes.data(), m_videoModes.size() };
		};

		Monitor(GLFWmonitor* handle) noexcept;
	private:

		static VideoMode convert_video_mode(const GLFWvidmode& mode) noexcept 
		{
			return VideoMode{
				.width {static_cast<uint32_t>(std::clamp(mode.width, 0, (1<<16) - 1))},
				.height {static_cast<uint32_t>(std::clamp(mode.height, 0, (1<<16) - 1))},
				.redBits {static_cast<uint32_t>(std::clamp(mode.redBits, 0, (1 << 8) - 1))},
				.greenBits {static_cast<uint32_t>(std::clamp(mode.greenBits, 0, (1 << 8) - 1))},
				.blueBits {static_cast<uint32_t>(std::clamp(mode.blueBits, 0, (1 << 8) - 1))},
				.refreshRate {static_cast<uint32_t>(std::clamp(mode.refreshRate, 0, (1<<16) - 1 ))}
			};
		}
		void init_video_modes() noexcept {
			::assert(t_mainThread);

			m_videoModes.clear();
			int count{ 0 };

			if (auto result = Error::create(glfwGetVideoModes, m_handle, &count); result) [[likely]] {
				m_videoModes.reserve(count);
				//nyan::log::info().message("[GLFW] Monitor detected: ").message(m_monitorName);
				for (auto vidMode : std::span{ *result, static_cast<size_t>(count) }) {
					m_videoModes.push_back(convert_video_mode(vidMode));
					const auto& mode = m_videoModes.back();
					//nyan::log::info().format("[GLFW]\t{}x{}@{}hz, {}|{}|{}", mode.width, mode.height, mode.refreshRate, mode.redBits, mode.greenBits, mode.blueBits);
				}
			}
		}
		void init_name() noexcept;
		void init_pos() noexcept {

			int xpos, ypos;
			if (auto result = Error::create(glfwGetMonitorPos, m_handle, &xpos, &ypos); !result) [[unlikely]]
				return;
			m_xPos = xpos;
			m_yPos = ypos;
		}

		GLFWmonitor* m_handle{ nullptr };
		std::vector<VideoMode> m_videoModes{};
		VideoMode m_desktopVideoMode{};
		std::string m_monitorName{};
		int32_t m_xPos{ 0 };
		int32_t m_yPos{ 0 };

		inline static thread_local bool t_mainThread{ false };
	};
	std::expected<Monitor::VideoMode, nyan::glfw::Error> Monitor::get_current_video_mode() noexcept
	{
		::assert(t_mainThread);
		if (auto result = Error::create(glfwGetVideoMode, m_handle); !result) [[unlikely]]
			return std::unexpected{ result.error() };
		else
			return convert_video_mode(*result.value());
	}
	Monitor::Monitor(GLFWmonitor* handle) noexcept : m_handle(handle)
	{
		init_pos();
		init_name();
		if (auto result = get_current_video_mode(); result)
			m_desktopVideoMode = result.value();
		init_video_modes();
	}
	void Monitor::init_name() noexcept
	{
		::assert(t_mainThread);


		if (auto result = Error::create(glfwGetMonitorName, m_handle); result) [[likely]]
			m_monitorName = *result;


#ifdef WIN32
		if (m_monitorName != "Generic PnP Monitor")
			return;
		auto adapter = glfwGetWin32Adapter(m_handle);
		DISPLAY_DEVICE dd;
		dd.cb = sizeof(dd);
		EnumDisplayDevices(adapter, 0, &dd, 1);
		//nyan::log::info().format("[WIN32] DD {}, {}, {}, {}", dd.DeviceName, dd.DeviceString, dd.DeviceID, dd.DeviceKey);
		m_monitorName = dd.DeviceString;

		if (m_monitorName != "Generic PnP Monitor")
			return;

		std::vector<DISPLAYCONFIG_PATH_INFO> paths;
		std::vector<DISPLAYCONFIG_MODE_INFO> modes;
		UINT32 flags = QDC_ONLY_ACTIVE_PATHS | QDC_VIRTUAL_MODE_AWARE;
		LONG result = ERROR_SUCCESS;

		do
		{
			// Determine how many path and mode structures to allocate
			UINT32 pathCount, modeCount;
			result = GetDisplayConfigBufferSizes(flags, &pathCount, &modeCount);

			if (result != ERROR_SUCCESS)
			{
				return;
			}

			// Allocate the path and mode arrays
			paths.resize(pathCount);
			modes.resize(modeCount);
			for (auto& mode : modes)
				mode.infoType = DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE;

			// Get all active paths and their modes
			result = QueryDisplayConfig(flags, &pathCount, paths.data(), &modeCount, modes.data(), nullptr);

			// The function may have returned fewer paths/modes than estimated
			paths.resize(pathCount);
			modes.resize(modeCount);

			// It's possible that between the call to GetDisplayConfigBufferSizes and QueryDisplayConfig
			// that the display state changed, so loop on the case of ERROR_INSUFFICIENT_BUFFER.
		} while (result == ERROR_INSUFFICIENT_BUFFER);

		if (result != ERROR_SUCCESS)
		{
			return;
		}

		// For each active path
		size_t pathIdx = 0;
		size_t modeIdx = std::numeric_limits<size_t>::max();
		for (const auto& mode : modes)
			if (mode.infoType == DISPLAYCONFIG_MODE_INFO_TYPE_SOURCE && mode.sourceMode.position.x == m_xPos && mode.sourceMode.position.y == m_yPos)
				modeIdx = mode.id;

		if (modeIdx == std::numeric_limits<size_t>::max())
			return;

		for (const auto& path : paths)
			if (path.sourceInfo.id == modeIdx)
				break;
			else
				pathIdx++;
		auto& path = paths[pathIdx];
		// Find the target (monitor) friendly name
		DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
		targetName.header.adapterId = path.targetInfo.adapterId;
		targetName.header.id = path.targetInfo.id;
		targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
		targetName.header.size = sizeof(targetName);
		result = DisplayConfigGetDeviceInfo(&targetName.header);

		if (result != ERROR_SUCCESS)
		{
			return;
		}

		// Find the adapter device name
		DISPLAYCONFIG_ADAPTER_NAME adapterName = {};
		adapterName.header.adapterId = path.targetInfo.adapterId;
		adapterName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_ADAPTER_NAME;
		adapterName.header.size = sizeof(adapterName);

		result = DisplayConfigGetDeviceInfo(&adapterName.header);

		if (result != ERROR_SUCCESS)
		{
			return;
		}

		//nyan::log::info().format("{}, {}\n{}\n{}", (targetName.flags.friendlyNameFromEdid ? convert_wchar_to_utf8(targetName.monitorFriendlyDeviceName) : "Unknown"), static_cast<int>(targetName.outputTechnology), convert_wchar_to_utf8(adapterName.adapterDevicePath), path.targetInfo.id);
		if (targetName.flags.friendlyNameFromEdid)
			m_monitorName = convert_wchar_to_utf8(targetName.monitorFriendlyDeviceName);
		return;
		
#endif
	}
}