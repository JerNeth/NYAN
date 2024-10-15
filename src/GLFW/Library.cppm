module;

//#include <cassert>
//#include <expected>
//#include <memory>
//#include <string_view>
//#include <span>
//#include <vector>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN  
#define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

export module NYANGLFW:Library; 
import std;
import NYANAssert;
import NYANLog;
import :Error;
import :Allocator;
import :Window;
import :Monitor;
import :ModuleSettings;

export namespace nyan::glfw
{
	class Library
	{
	public:
		struct WindowParams {
			uint32_t width {1920};
			uint32_t height {1080};
			const char* windowTitle{ "NYAN" };
			bool resizable{ true };
			bool visible{ true };
			bool decorated{ true };
			bool focused{ true };
			bool autoIconify{ true };
			bool floating{ false };
			bool maximized{ false };
			bool centerCursor{ true };
			int posX{ static_cast<int>(GLFW_ANY_POSITION) };
			int posY{ static_cast<int>(GLFW_ANY_POSITION) };
			uint32_t refreshRate{ static_cast<uint32_t>(GLFW_DONT_CARE )};
		};
	public:
		~Library() noexcept;
		Library(Library&) = delete;
		Library(Library&&) noexcept;

		Library& operator=(Library&) = delete;
		Library& operator=(Library&&) noexcept;

		void poll_events() noexcept;

		[[nodiscard]] std::expected<Window, Error> create_window(const WindowParams& params) const noexcept;

		[[nodiscard]] std::expected<Window, Error> create_fullscreen_window(const WindowParams& params, Monitor& monitor) const noexcept;

		[[nodiscard]] bool vulkan_supported() const noexcept;

		[[nodiscard]] bool blocks() const noexcept;

		[[nodiscard]] static std::expected<Library, Error> create() noexcept;

		[[nodiscard]] std::span<const Monitor> monitors() const noexcept;
	private:
		void init_monitors() noexcept;
		Library(std::unique_ptr<Allocator> allocator) noexcept;

		bool m_initialized { false };
		std::atomic<bool> m_blocking{ false };
		std::unique_ptr<Allocator> m_allocator{ nullptr };
		std::vector<Monitor> m_monitors{};

		inline static thread_local bool t_mainThread{ false };
	};
}
namespace nyan::glfw
{
	Library::~Library() noexcept
	{
		::assert(t_mainThread);
		if (m_initialized)
			glfwTerminate();
	}

	Library::Library(Library&& other) noexcept :
		m_initialized(std::exchange(other.m_initialized, false)),
		m_allocator(std::exchange(other.m_allocator, nullptr))
	{
	}

	Library& Library::operator=(Library&& other) noexcept
	{
		if (std::addressof(other) != this) [[unlikely]] {
			std::swap(m_initialized, other.m_initialized);
			std::swap(m_allocator, other.m_allocator);
		}
		return *this;
	}

	void Library::poll_events() noexcept
	{
		::assert(t_mainThread);
		m_blocking.store(true, std::memory_order_release);
		glfwPollEvents();
		m_blocking.store(false, std::memory_order_release);
	}

	std::expected<Window, Error> Library::create_window(const WindowParams& params) const noexcept
	{
		::assert(t_mainThread);

		if (auto result = Error::create(glfwWindowHint, GLFW_CLIENT_API, GLFW_NO_API); !result) [[unlikely]]
			return std::unexpected{ result.error()};

		nyan::ignore = Error::create(glfwWindowHint, GLFW_RESIZABLE, static_cast<int>(params.resizable));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_VISIBLE, static_cast<int>(params.visible));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_DECORATED, static_cast<int>(params.decorated));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_FOCUSED, static_cast<int>(params.focused));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_AUTO_ICONIFY, static_cast<int>(params.autoIconify));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_FLOATING, static_cast<int>(params.floating));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_MAXIMIZED, static_cast<int>(params.maximized));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_CENTER_CURSOR, static_cast<int>(params.centerCursor));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_POSITION_X, static_cast<int>(params.posX));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_POSITION_Y, static_cast<int>(params.posY));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_REFRESH_RATE, static_cast<int>(params.refreshRate));

		if (auto result = Error::create(glfwCreateWindow, static_cast<int>(params.width), static_cast<int>(params.height), params.windowTitle, static_cast<GLFWmonitor*>(nullptr), static_cast<GLFWwindow*>(nullptr)); !result) [[unlikely]]
			return std::unexpected{ result.error() };
		else
			return Window{*result, params.width , params.height };
	}

	std::expected<Window, Error> Library::create_fullscreen_window(const WindowParams& params, Monitor& monitor) const noexcept
	{
		::assert(t_mainThread);

		if (auto result = Error::create(glfwWindowHint, GLFW_CLIENT_API, GLFW_NO_API); !result) [[unlikely]]
			return std::unexpected{ result.error() };

		nyan::ignore = Error::create(glfwWindowHint, GLFW_RESIZABLE, static_cast<int>(params.resizable));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_VISIBLE, static_cast<int>(params.visible));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_DECORATED, static_cast<int>(params.decorated));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_FOCUSED, static_cast<int>(params.focused));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_AUTO_ICONIFY, static_cast<int>(params.autoIconify));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_FLOATING, static_cast<int>(params.floating));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_MAXIMIZED, static_cast<int>(params.maximized));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_CENTER_CURSOR, static_cast<int>(params.centerCursor));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_POSITION_X, static_cast<int>(params.posX));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_POSITION_Y, static_cast<int>(params.posY));
		nyan::ignore = Error::create(glfwWindowHint, GLFW_REFRESH_RATE, static_cast<int>(params.refreshRate));

		if (auto result = Error::create(glfwCreateWindow, static_cast<int>(params.width), static_cast<int>(params.height), params.windowTitle, monitor.m_handle, static_cast<GLFWwindow*>(nullptr)); !result) [[unlikely]]
			return std::unexpected{ result.error() };
		else
			return Window{ *result, params.width , params.height };
	}

	bool Library::vulkan_supported() const noexcept
	{
		return glfwVulkanSupported();
	}

	bool Library::blocks() const noexcept
	{
		return m_blocking.load(std::memory_order_consume);
	}

	std::expected<Library, Error> Library::create() noexcept
	{
		constexpr bool useDebugAllocator = true;
		nyan::log::info().format("Using GLFW version: {}", glfwGetVersionString());
		auto allocator = std::make_unique<Allocator>();

		if(useDebugAllocator)
			glfwInitAllocator(allocator->get_allocator());

		if (auto result = Error::create(glfwInit); !result) [[unlikely]]
			return std::unexpected{ result.error()};

		//GLFW assumes some calls are made from the main thread only
		//Here, we assume the thread that calls init is also the main thread
		t_mainThread = true;
		Monitor::t_mainThread = true;
		Window::t_mainThread = true;

		
		return Library{ std::move(allocator) };
	}

	std::span<const Monitor> Library::monitors() const noexcept
	{
		return {m_monitors.data(), m_monitors.size()};
	}

	void Library::init_monitors() noexcept
	{
		::assert(t_mainThread);
		m_monitors.clear();
		int monitorCount{ 0 };
		if (auto result = Error::create(glfwGetMonitors, &monitorCount); !result) [[unlikely]] {
			return;
		}
		else {
			m_monitors.reserve(monitorCount);
			for (int i = 0; i < monitorCount; ++i) {
				m_monitors.emplace_back(result.value()[i]);
			}
		}


	}

	Library::Library(std::unique_ptr<Allocator> allocator) noexcept :
		m_initialized(true),
		m_allocator(std::move(allocator))
	{
		init_monitors();
	}
}