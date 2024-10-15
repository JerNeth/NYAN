module;

//#include <expected>

export module NYANRenderer:WSI;
import std;

import NYANVulkan;
import NYANGLFW;

import :Error;

export namespace nyan::renderer
{
	class WSI
	{
	public:
		void acquire() noexcept;
		void present() noexcept;
		[[nodiscard("must handle potential error")]] static std::expected<WSI, WSIError> create(glfw::Window& r_window) noexcept;
	private:
		WSI() noexcept;

		//glfw::Window& r_window;
		//Surface surface;
		//Swapchain swapchain;
	};
}
