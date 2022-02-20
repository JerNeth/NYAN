#ifndef CONTEXT_INIT_H
#define CONTEXT_INIT_H
#pragma once

#include "imgui.h"
#include <vector>

namespace glfww {
	class Library {
	public:
		Library() {
			if (glfwInit() == GLFW_FALSE)
				throw std::runtime_error("GLFW could not be initiated");
		}
		Library(std::vector<std::pair<int, int>> hints) {
			for (auto it : hints) {
				glfwInitHint(it.first, it.second);
				const char* error_msg;
				int error = glfwGetError(&error_msg);
				if (error) {
					if (error == GLFW_INVALID_ENUM)
						std::cerr << "Invalid Enum: " << error_msg;
					if (error == GLFW_INVALID_VALUE)
						std::cerr << "Invalid Value: " << error_msg;
				}
			}
			if (glfwInit() == GLFW_FALSE)
				throw std::runtime_error("GLFW could not be initiated");
		}
		[[nodiscard]] std::vector<const char*> get_required_extensions() const {
			uint32_t glfwExtensionCount = 0;
			const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
			std::vector<const char*> ret;
			ret.reserve(glfwExtensionCount);
			for (uint32_t i = 0; i < glfwExtensionCount; i++)
				ret.push_back(glfwExtensions[i]);
			#ifdef X_PROTOCOL
			ret.push_back(VK_KHR_XLIB_SURFACE_EXTENSION_NAME);
			#endif
			return ret;
		}
		~Library() {
			glfwTerminate();
		}
		// No reason to keep these
		Library(const Library&) = delete;
		Library(Library&& other) = delete;
		Library& operator=(const Library&) = delete;
		Library& operator=(Library&& other) = delete;
	};
	enum class WindowMode {
		Windowed,
		FullscreenWindowed,
		Fullscreen,
		Size
	};
	class Monitor {
	public:
		explicit Monitor(GLFWmonitor* monitor = nullptr) :
			m_monitor(monitor == nullptr? glfwGetPrimaryMonitor() : monitor) {
			if (!m_monitor)
				throw std::runtime_error("GLFW: Couldn't find monitor");
			int count;
			auto l = glfwGetVideoModes(m_monitor, &count);
			m_modes.reserve(count);
			for (int i = 0; i < count; i++) {
				m_modes.push_back(l[i]);
			}
			const auto* mode = glfwGetVideoMode(m_monitor);
			if (!mode)
				throw std::runtime_error("GLFW: Couldn't find video mode for monitor");
			m_defaultMode = *mode;

			const auto* name = glfwGetMonitorName(m_monitor);
			if (!name)
				throw std::runtime_error("GLFW: Couldn't find name for monitor");
			m_name = name;
#ifdef GLFW_EXPOSE_NATIVE_WIN32
			m_identifier = glfwGetWin32Monitor(m_monitor);
#else 
			m_identifier = m_name;
#endif
		}
		const GLFWvidmode* get_default_mode() const {
			return &m_defaultMode;
		}
		operator GLFWmonitor* () const {
			return m_monitor;
		}
		std::pair<int, int> get_default_extent() const {
			return {m_defaultMode.width, m_defaultMode.height};
		}
		const std::string& get_name() const {
			return m_name;
		}
		const std::string& get_identifier() const {
			return m_identifier;
		}
	private:
		GLFWmonitor* m_monitor = nullptr;
		std::string m_name;
		std::string m_identifier;
		std::vector<GLFWvidmode> m_modes;
		GLFWvidmode m_defaultMode;
	};
	static std::vector<Monitor> get_monitors() {
		int count;
		std::vector<Monitor> monit;
		auto monitors = glfwGetMonitors(&count);
		for (int i = 0; i < count; i++) {
			monit.emplace_back(monitors[i]);
		}
		return monit;
	}
	class Window {
		static const char* ImGui_ImplGlfw_GetClipboardText(void* user_data)
		{
			return glfwGetClipboardString((GLFWwindow*)user_data);
		}

		static void ImGui_ImplGlfw_SetClipboardText(void* user_data, const char* text)
		{
			glfwSetClipboardString((GLFWwindow*)user_data, text);
		}

	public:
		Window() = delete;
		Window(int width, int height, const Monitor& monitor, WindowMode mode, const std::string& windowTitle) {
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			ptr_monitor = &monitor;
			m_mode = mode;
			switch (mode) {
			case WindowMode::Windowed: {
				m_width = width;
				m_height = height;
				m_window = glfwCreateWindow(m_width, m_height, windowTitle.c_str(), nullptr, nullptr);
				break;
			}
			case WindowMode::FullscreenWindowed: {
				auto mode_ = monitor.get_default_mode();
				m_width = mode_->width;
				m_height = mode_->height;
				glfwWindowHint(GLFW_RED_BITS, mode_->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode_->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode_->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode_->refreshRate);
				m_window = glfwCreateWindow(m_width, m_height, windowTitle.c_str(), monitor, nullptr);
				break;
			}
			case WindowMode::Fullscreen: {
				m_width = width;
				m_height = height;
				m_window = glfwCreateWindow(m_width, m_height, windowTitle.c_str(), monitor, nullptr);
				break;
			}
			}
			if (!glfwVulkanSupported())
			{
				throw std::runtime_error("GLFW: Vulkan not supported");
			}
			
			if (!m_window) {
				const char* error_msg;
				glfwGetError(&error_msg);
				throw std::runtime_error(error_msg);
			}
		}
		void change_mode(WindowMode mode) {
			//assert(false); //
			if (mode == m_mode)
				return;
			m_mode = mode;
			switch (mode) {
			case WindowMode::Windowed: {
				auto [width, height] = ptr_monitor->get_default_extent();
				auto x = (width - m_width) / 2;
				auto y = (height - m_height) / 2;
				glfwSetWindowMonitor(m_window, nullptr, x, y, m_width, m_height, GLFW_DONT_CARE);
				break;
			}
			case WindowMode::FullscreenWindowed: {
				auto mode_ = ptr_monitor->get_default_mode();
				glfwWindowHint(GLFW_RED_BITS, mode_->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode_->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode_->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode_->refreshRate);
				glfwSetWindowMonitor(m_window, *ptr_monitor, 0, 0, mode_->width, mode_->height, mode_->refreshRate);
				break;
			}
			case WindowMode::Fullscreen: {
				glfwSetWindowMonitor(m_window, *ptr_monitor, 0, 0, m_width, m_height, GLFW_DONT_CARE);
				break;
				}
			}
		}
		void resize(int width, int height) {
			//assert(false);
			m_width = width;
			m_height = height;
			switch (m_mode) {
			case WindowMode::Windowed:
				m_width = width;
				m_height = height;
				glfwSetWindowSize(m_window, m_width, m_height);
				break;
			case WindowMode::FullscreenWindowed:
				assert(false);
				break;
			case WindowMode::Fullscreen:
				m_width = width;
				m_height = height;
				glfwSetWindowSize(m_window, m_width, m_height);
				break;
			}
		}
		bool is_iconified() {
			int iconified = glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
			return iconified != 0;
		}
#if defined(_WIN32)
		inline HWND get_win32_window() {
			return glfwGetWin32Window(m_window);
		}
#else
		inline Display* get_x11_display() {
			return glfwGetX11Display();
		}
		inline ::Window get_x11_window() {
			return glfwGetX11Window(m_window);
		}
#endif
		~Window() {
			glfwDestroyWindow(m_window);
		}
		Window(const Window&) = delete;
		Window(Window&& other) noexcept {
			this->m_window = other.m_window;
			other.m_window = nullptr;
		}
		Window& operator=(const Window&) = delete;
		Window& operator=(Window&& other) noexcept {
			this->m_window = other.m_window;
			other.m_window = nullptr;
			return *this;
		}
		inline bool should_close() {
			return glfwWindowShouldClose(m_window);
		}
		void swap_buffers() {
			glfwSwapBuffers(m_window);
			const char* error_msg;
			int error = glfwGetError(&error_msg);
			if(error != 0)
				throw std::runtime_error(error_msg);
		
		}
		void configure_imgui() {
			auto& io = ImGui::GetIO();
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
			io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
			io.BackendPlatformName = "imgui_impl_glfw";

			// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
			io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
			io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
			io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
			io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
			io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
			io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
			io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
			io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
			io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
			io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
			io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
			io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
			io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
			io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
			io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
			io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
			io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
			io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

			io.SetClipboardTextFn = ImGui_ImplGlfw_SetClipboardText;
			io.GetClipboardTextFn = ImGui_ImplGlfw_GetClipboardText;
			io.ClipboardUserData = m_window;
			#if defined(_WIN32)
			io.ImeWindowHandle = (void*)glfwGetWin32Window(m_window);
			#endif
			glfwSetKeyCallback(m_window, []([[maybe_unused]] GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, [[maybe_unused]] int mods) {
				ImGuiIO& io = ImGui::GetIO();
				if (action == GLFW_PRESS)
					io.KeysDown[key] = true;
				if (action == GLFW_RELEASE)
					io.KeysDown[key] = false;

				// Modifiers are not reliable across systems
				io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
				io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
				io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
				#ifdef _WIN32
				io.KeySuper = false;
				#else
				io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
				#endif
			});
			glfwSetScrollCallback(m_window, []([[maybe_unused]] GLFWwindow* window, double xoffset, double yoffset)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.MouseWheelH += (float)xoffset;
				io.MouseWheel += (float)yoffset;
			});
			glfwSetCharCallback(m_window, []([[maybe_unused]] GLFWwindow* window, unsigned int c)
			{
				ImGuiIO& io = ImGui::GetIO();
				io.AddInputCharacter(c);
			});
		}
		void imgui_update_mouse_keyboard() {
			ImGuiIO& io = ImGui::GetIO();
			for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++)
			{
				// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
				io.MouseDown[i] = glfwGetMouseButton(m_window, i) != 0;
			}
			const ImVec2 mouse_pos_backup = io.MousePos;
			io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			const bool focused = glfwGetWindowAttrib(m_window, GLFW_FOCUSED) != 0;
			if (focused)
			{
				if (io.WantSetMousePos)
				{
					glfwSetCursorPos(m_window, (double)mouse_pos_backup.x, (double)mouse_pos_backup.y);
				}
				else
				{
					double mouse_x, mouse_y;
					glfwGetCursorPos(m_window, &mouse_x, &mouse_y);
					io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
				}
			}
		}
	private:
		GLFWwindow* m_window = nullptr;
		const Monitor* ptr_monitor = nullptr;
		int m_width = 0;
		int m_height = 0;
		WindowMode m_mode = WindowMode::Windowed;
	};

}
#endif // !CONTEXT_INIT_H