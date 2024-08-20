module;

#include <utility>
#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:Object;
import :LogicalDeviceWrapper;
import :Error;

export namespace nyan::vulkan::wrapper
{
	//class LogicalDeviceWrapper;
	template<typename HandleClass>
	class Object
	{
	public:
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
			: r_device(device),
			  m_handle(VK_NULL_HANDLE)
		{

		}
		constexpr Object(const LogicalDeviceWrapper& device, const HandleClass& handle) noexcept :
			r_device(device),
			m_handle(handle)
		{

		}
		constexpr Object(Object&& other) noexcept :
			r_device(other.r_device),
			m_handle(other.m_handle)
		{
			other.m_handle = VK_NULL_HANDLE;
		}
		constexpr Object& operator=(Object&& other) noexcept
		{
			if (this != &other)
			{
				assert(&r_device == &other.r_device);
				std::swap(m_handle, other.m_handle);
				// m_handle = other.m_handle;
				// other.m_handle = VK_NULL_HANDLE;
			}
			return *this;
		}
		const LogicalDeviceWrapper& r_device;
		HandleClass m_handle{ VK_NULL_HANDLE };
	};
}
