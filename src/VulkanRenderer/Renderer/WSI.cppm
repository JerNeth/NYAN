module;

//#include <expected>

export module NYANRenderer:WSI;
import std;

import NYANVulkan;
import NYANSDL;

import :Error;

export namespace nyan::renderer
{
	class WSI
	{
	public:
		void acquire() noexcept;
		void present() noexcept;
		[[nodiscard("must handle potential error")]] static std::expected<WSI, WSIError> create() noexcept;
	private:
		WSI() noexcept;

		//glfw::Window& r_window;
		//Surface surface;
		//Swapchain swapchain;
	};
}
