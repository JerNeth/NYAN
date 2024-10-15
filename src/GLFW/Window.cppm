module;
//
//#include <c::assert>
//#include <expected>
//#include <utility>
//#include <string_view>

#include <GLFW/glfw3.h>

#ifdef WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN  
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

export module NYANGLFW:Window;
import std;

import NYANLog;
import NYANData;
import NYANAssert;

import :Error;
import :Monitor;
import :ModuleSettings;


export namespace nyan::glfw
{
	class Window
	{
	public:
		struct Resolution {
			uint32_t width;
			uint32_t height;
		};
		struct PlatformWindowData {
			void* hwnd;
			void* hinstance;
		};
		friend class Library;

		[[nodiscard]] bool is_focused() noexcept {
			return m_focused.load(std::memory_order_consume);
		};
		[[nodiscard]] bool is_iconified() noexcept {
			return m_iconified.load(std::memory_order_consume);
		};
		[[nodiscard]] bool is_maximized() noexcept {
			return m_maximized.load(std::memory_order_consume);
		};
		[[nodiscard]] bool is_hovered() noexcept {
			::assert(t_mainThread);
			if (auto result = Error::create(glfwGetWindowAttrib, m_handle, GLFW_HOVERED); result)
				return *result;
			return false;
		};
		[[nodiscard]] bool is_visible() noexcept {
			::assert(t_mainThread);
			if (auto result = Error::create(glfwGetWindowAttrib, m_handle, GLFW_VISIBLE); result)
				return *result;
			return false;
		};
		[[nodiscard]] bool should_close() noexcept {
			return m_shouldClose.load(std::memory_order_consume);
		};
		[[nodiscard]] std::expected<Resolution, Error> get_resolution() noexcept {
			Resolution res;
			if (auto result = Error::create(glfwGetWindowSize, m_handle, reinterpret_cast<int*>(&res.width), reinterpret_cast<int*>(&res.height)); !result)
				return std::unexpected{ result.error() };
			return res;
		};
		void set_should_close(bool shouldClose) noexcept {
			nyan::ignore = Error::create(glfwSetWindowShouldClose, m_handle, static_cast<int>(shouldClose));
		};
		void request_attention() noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwRequestWindowAttention, m_handle);
		}

		void set_resolution(Resolution resolution) noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowSize, m_handle, static_cast<int>(resolution.width), static_cast<int>(resolution.height));
		}

		void set_visible(bool value) noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowAttrib, m_handle, GLFW_VISIBLE, static_cast<int>(value));
		}

		void set_resizable(bool value) noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowAttrib, m_handle, GLFW_RESIZABLE, static_cast<int>(value));
		}

		void set_decorated(bool value) noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowAttrib, m_handle, GLFW_DECORATED, static_cast<int>(value));
		}

		void set_monitor(Monitor& monitor, Resolution resolution, uint32_t refreshRate = GLFW_DONT_CARE) noexcept {
			static_assert(static_cast<int>(static_cast<uint32_t>(GLFW_DONT_CARE)) == GLFW_DONT_CARE);
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowMonitor, m_handle, monitor.m_handle, 0, 0, static_cast<int>(resolution.width), static_cast<int>(resolution.height), static_cast<int>(refreshRate));
		}

		void set_title(const char* title) noexcept {
			::assert(t_mainThread);
			nyan::ignore = Error::create(glfwSetWindowTitle, m_handle, title);
		}

		[[nodiscard]] std::expected < PlatformWindowData, Error> get_platform_data() noexcept
		{
			PlatformWindowData platformData;
#ifdef WIN32
			if (auto result = Error::create(glfwGetWin32Window, m_handle); !result)
				return std::unexpected{ result.error() };
			else
				platformData.hwnd = *result;
			platformData.hinstance = GetModuleHandle(nullptr);
			return platformData;
#endif
			return std::unexpected{ Error{0} };
		}



		~Window() noexcept 
		{
			::assert(t_mainThread);
			if (m_handle)
				nyan::ignore = Error::create(glfwDestroyWindow, m_handle);
		}

		Window(Window&) = delete;
		Window(Window&& other) noexcept : m_handle(std::exchange(other.m_handle, nullptr)) 
		{
			::assert(t_mainThread);
			if (auto result = Error::create(glfwSetWindowUserPointer, m_handle, static_cast<void*>(this)); !result)
				return;
		}

		Window& operator=(Window&) = delete;
		Window& operator=(Window&& other) noexcept 
		{
			if (std::addressof(other) != this)
			{
				std::swap(m_handle, other.m_handle);
				::assert(t_mainThread);
				nyan::ignore = Error::create(glfwSetWindowUserPointer, m_handle, static_cast<void*>(this));
				nyan::ignore = Error::create(glfwSetWindowUserPointer, other.m_handle, static_cast<void*>(std::addressof(other)));
			}
			return *this;
		}

	private:
		void set_callbacks() noexcept {
			::assert(t_mainThread);
			if (auto result = Error::create(glfwSetWindowUserPointer, m_handle, static_cast<void*>(this)); !result)
				return;

			if (auto result = Error::create(glfwSetWindowSizeCallback, m_handle, [](GLFWwindow* window, int width, int height) {
				Window* handle{ nullptr };
				if (auto result = Error::create(glfwGetWindowUserPointer, window); !result)
					return;
				else
					handle = reinterpret_cast<Window*>(*result);

				::assert(width >= 0);
				::assert(height >= 0);
				handle->resize(static_cast<uint32_t>(width), static_cast<uint32_t>(height));
				}); !result)
				return;

			if (auto result = Error::create(glfwSetWindowMaximizeCallback, m_handle, [](GLFWwindow* window, int maximized) {
				Window* handle{ nullptr };
				if (auto result = Error::create(glfwGetWindowUserPointer, window); !result)
					return;
				else
					handle = reinterpret_cast<Window*>(*result);

				::assert(maximized == GLFW_TRUE || maximized == GLFW_FALSE);
				handle->maximize(maximized == GLFW_TRUE);
				}); !result)
				return;

			if (auto result = Error::create(glfwSetWindowIconifyCallback, m_handle, [](GLFWwindow* window, int iconified) {
				Window* handle{ nullptr };
				if (auto result = Error::create(glfwGetWindowUserPointer, window); !result)
					return;
				else
					handle = reinterpret_cast<Window*>(*result);

				::assert(iconified == GLFW_TRUE || iconified == GLFW_FALSE);
				handle->iconify(iconified == GLFW_TRUE);
				}); !result)
				return;

			if (auto result = Error::create(glfwSetWindowFocusCallback, m_handle, [](GLFWwindow* window, int focused) {
				Window* handle{ nullptr };
				if (auto result = Error::create(glfwGetWindowUserPointer, window); !result)
					return;
				else
					handle = reinterpret_cast<Window*>(*result);

				::assert(focused == GLFW_TRUE || focused == GLFW_FALSE);
				handle->focus(focused == GLFW_TRUE);
				}); !result)
				return;

			if (auto result = Error::create(glfwSetWindowCloseCallback, m_handle, [](GLFWwindow* window) {
				Window* handle{ nullptr };
				if (auto result = Error::create(glfwGetWindowUserPointer, window); !result)
					return;
				else
					handle = reinterpret_cast<Window*>(*result);

				handle->close();
				}); !result)
				return;

		}

		void resize(uint32_t width, uint32_t height) noexcept {
			::assert(m_handle);
			m_width.store(width, std::memory_order_release);
			m_height.store(height, std::memory_order_release);
			nyan::log::verbose().format("[GLFW] resize event {}x{}", width, height);
		}

		void maximize(bool state) noexcept {
			::assert(m_handle);
			m_maximized.store(state, std::memory_order_release);
			nyan::log::verbose().format("[GLFW] maximize event {}", state);
		}

		void iconify(bool state) noexcept {
			::assert(m_handle);
			m_iconified.store(state, std::memory_order_release);
			nyan::log::verbose().format("[GLFW] iconify event {}", state);
		}

		void focus(bool state) noexcept {
			::assert(m_handle);
			m_focused.store(state, std::memory_order_release);
			nyan::log::verbose().format("[GLFW] focus event {}", state);
		}

		void close() noexcept {
			::assert(m_handle);
			m_shouldClose.store(true, std::memory_order_release);
			nyan::log::verbose().format("[GLFW] close event");
		}

		Window(GLFWwindow* handle, uint32_t width, uint32_t height) noexcept 
			: m_handle(handle) ,
			m_width(width),
			m_height(height)
		{
			set_callbacks();

			::assert(t_mainThread);
			if (auto result = Error::create(glfwGetWindowAttrib, m_handle, GLFW_MAXIMIZED); result)
				m_maximized.store(*result == GLFW_TRUE, std::memory_order_release);			
			else
				::assert(false);
			if (auto result = Error::create(glfwGetWindowAttrib, m_handle, GLFW_ICONIFIED); result)
				m_iconified.store(*result == GLFW_TRUE, std::memory_order_release);			
			else
				::assert(false);
			if (auto result = Error::create(glfwGetWindowAttrib, m_handle, GLFW_FOCUSED); result)
				m_focused.store(*result == GLFW_TRUE, std::memory_order_release);
			else
				::assert(false);

			Resolution res;

			if (auto result = Error::create(glfwGetWindowSize, m_handle, reinterpret_cast<int*>(&res.width), reinterpret_cast<int*>(&res.height)); result) {
				m_width.store(res.width, std::memory_order_release);
				m_height.store(res.height, std::memory_order_release);
			}
			else {
				::assert(false);
			}
				
		};

		GLFWwindow* m_handle{ nullptr };
		std::atomic<uint32_t> m_width{0};
		std::atomic<uint32_t> m_height{0};
		std::atomic<bool> m_maximized{ false };
		std::atomic<bool> m_iconified{ false };
		std::atomic<bool> m_focused{ false };
		std::atomic<bool> m_shouldClose{ false };
		inline static thread_local bool t_mainThread{ false };
	};
}