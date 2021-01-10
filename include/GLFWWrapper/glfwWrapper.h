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