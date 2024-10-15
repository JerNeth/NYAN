module;

//#include <utility>
//#include <expected>

#include "volk.h"

export module NYANVulkan:Object;
import std;
import :LogicalDeviceWrapper;
import :Error;

export namespace nyan::vulkan
{
	class LogicalDevice;
	template<typename HandleClass>
	class Object
	{
	public:
		friend class LogicalDevice;
		enum class Location : uint8_t {
			Device,
			Host
		};
		[[nodiscard]] const HandleClass& get_handle() const noexcept
		{
			return m_handle;
		}
		[[nodiscard("must handle potential error")]] std::expected<void, Error> set_debug_label(const char* name) const noexcept;
	protected:
		explicit constexpr Object(const LogicalDeviceWrapper& device) noexcept
			: ptr_device(&device),
			  m_handle(VK_NULL_HANDLE)
		{

		}
		constexpr Object(const LogicalDeviceWrapper& device, const HandleClass& handle) noexcept :
			ptr_device(&device),
			m_handle(handle)
		{

		}
		constexpr Object(Object&& other) noexcept :
			ptr_device(std::exchange(other.ptr_device, nullptr)),
			m_handle(std::exchange(other.m_handle, VK_NULL_HANDLE))
		{
		}
		constexpr Object& operator=(Object&& other) noexcept
		{
			if (this != &other)
			{
				std::swap(m_handle, other.m_handle);
				std::swap(ptr_device, other.ptr_device);
				// m_handle = other.m_handle;
				// other.m_handle = VK_NULL_HANDLE;
			}
			return *this;
		}
		const LogicalDeviceWrapper* ptr_device{ nullptr };
		HandleClass m_handle{ VK_NULL_HANDLE };
	};
}
