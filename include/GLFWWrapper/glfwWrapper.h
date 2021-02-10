#ifndef CONTEXT_INIT_H
#define CONTEXT_INIT_H
#pragma once
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
		~Library() {
			glfwTerminate();
		}
		// No reason to keep these
		Library(const Library&) = delete;
		Library(Library&& other) = delete;
		Library& operator=(const Library&) = delete;
		Library& operator=(Library&& other) = delete;
	};
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
		Window(int width = 800, int height = 600,GLFWmonitor* monitor = nullptr, const GLFWvidmode* mode = nullptr, const char* title = "Simple Engine") {
			if (mode != nullptr) {
				width = mode->width;
				height = mode->height;
				glfwWindowHint(GLFW_RED_BITS, mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
				//glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
			}
			if (!glfwVulkanSupported())
			{
				throw std::runtime_error("GLFW: Vulkan not supported");
			}
			//Because we are using vulkan
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			
			//TODO handle resizing
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
			//glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
			m_window = glfwCreateWindow(width, height, title, monitor, nullptr);
			
			if (!m_window) {
				const char* error_msg;
				glfwGetError(&error_msg);
				throw std::runtime_error(error_msg);
			}
		}
		bool is_iconified() {
			int iconified = glfwGetWindowAttrib(m_window, GLFW_ICONIFIED);
			return iconified != 0;
		}
		inline HWND get_win32_window() {
			return glfwGetWin32Window(m_window);
		}
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
			ImGuiIO& io = ImGui::GetIO();
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

		static Window create_full_screen(int width, int height) {
			return Window(width, height, glfwGetPrimaryMonitor());
		}
		static Window create_windowed_full_screen() {
			return Window(0, 0, nullptr, glfwGetVideoMode(glfwGetPrimaryMonitor()));
		}
	private:
		//No unique ptr because I'm not really sure what exactely this struct is
		GLFWwindow* m_window = nullptr;
	};

}
#endif // !CONTEXT_INIT_H