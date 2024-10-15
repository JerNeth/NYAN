module;

//#include <expected>
//#include <string_view>

#include <GLFW/glfw3.h>


export module NYANGLFW:Error; 
import std;
import NYANLog;
import NYANAssert;
import :ModuleSettings;


export namespace nyan::glfw
{
	class Error
	{
	public:
		Error(int code) noexcept;


		//Creates implicit <string_view> dependency
		template<typename Fun, class... Args>
		[[nodiscard]] static std::expected<std::invoke_result_t<Fun, Args...>, Error> create(Fun fun, Args... args) noexcept  {

			if constexpr (!std::is_same_v<std::invoke_result_t<Fun, Args...>, void>) {
				auto result = fun(args...);
				const char* msg{ nullptr };
				auto code = glfwGetError(&msg);
				if (msg) [[unlikely]]
					nyan::log::error().format("GLFW Error Message: {}", msg);
				if (code != GLFW_NO_ERROR) [[unlikely]]
					return std::unexpected{ Error{code} };
				return result;
			}
			else {
				fun(args...);
				const char* msg{ nullptr };
				auto code = glfwGetError(&msg);
				if (msg) [[unlikely]]
					nyan::log::error().format("GLFW Error Message: {}", msg);
				if (code != GLFW_NO_ERROR) [[unlikely]]
					return std::unexpected{ Error{code} };
				return {};
			}
		}
	};
}
namespace nyan::glfw
{
	Error::Error(int code) noexcept
	{
	}
}