module;

#include <source_location>

#include "volk.h"

export module NYANVulkanWrapper:Error;

export namespace nyan::vulkan::wrapper
{
	class Error
	{
	public:
		constexpr Error(VkResult result, const std::source_location& location = std::source_location::current()) noexcept;
		constexpr explicit operator VkResult() const noexcept;
		[[nodiscard]] constexpr VkResult get_result() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		VkResult m_result{ VK_SUCCESS };
		//const char* m_message{ "" };
	};
	class InstanceCreationError
	{
	public:
		enum class Type : uint32_t
		{
			VolkInitializationError,
			RequiredExtensionNotPresentError,
			OutOfMemoryError,
			APIVersionNotSupportedError,
			IncompatibleDriverError,
			PhysicalDeviceEnumerationError,
			UnknownError
		};
		constexpr InstanceCreationError(Type type, const std::source_location& location = std::source_location::current()) noexcept;
		[[nodiscard]] constexpr Type get_type() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		Type m_type{ };
		//const char* m_message{ "" };
	};
	class PhysicalDeviceCreationError
	{
	public:
		enum class Type : uint32_t
		{
			OutOfMemoryError,
			LayerNotPresent,
			UnknownError
		};
		constexpr PhysicalDeviceCreationError(Type type, const std::source_location& location = std::source_location::current()) noexcept;
		[[nodiscard]] constexpr Type get_type() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		Type m_type{ };
		//const char* m_message{ "" };
	};
	class LogicalDeviceCreationError
	{
	public:
		enum class Type : uint32_t
		{
			DeviceLostError,
			AllocatorCreationError,
			TooManyObjects,
			InitializationFailed,
			OutOfMemoryError,
			NoQueueSpecified,
			UnknownError
		};
		constexpr LogicalDeviceCreationError(Type type, const std::source_location& location = std::source_location::current()) noexcept;
		[[nodiscard]] constexpr Type get_type() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		Type m_type{ };
		//const char* m_message{ "" };
	};
	class PhysicalDeviceSelectionError
	{
	public:
		enum class Type : uint32_t
		{
			NoPhysicalDeviceFoundError,
			NoValidPhysicalDeviceError
		};
		constexpr PhysicalDeviceSelectionError(Type type, const std::source_location& location = std::source_location::current()) noexcept;
		[[nodiscard]] constexpr Type get_type() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		Type m_type{ };
		//const char* m_message{ "" };
	};

}

constexpr nyan::vulkan::wrapper::Error::Error(const VkResult result, const std::source_location& location) noexcept :
	m_result(result)
	//m_message("")
{
}


constexpr nyan::vulkan::wrapper::Error::operator VkResult() const noexcept
{
	return get_result();
}

constexpr VkResult nyan::vulkan::wrapper::Error::get_result() const noexcept
{
	return m_result;
}
constexpr const char* nyan::vulkan::wrapper::Error::get_message() const noexcept
{
	//return m_message;
	return nullptr;
}

constexpr nyan::vulkan::wrapper::InstanceCreationError::InstanceCreationError(Type type,
	const std::source_location& location) noexcept :
	m_type(type)
{
}

constexpr nyan::vulkan::wrapper::InstanceCreationError::Type nyan::vulkan::wrapper::InstanceCreationError::get_type() const noexcept
{
	return m_type;
}

constexpr const char* nyan::vulkan::wrapper::InstanceCreationError::get_message() const noexcept
{
	//return m_message;
	return nullptr;
}

constexpr nyan::vulkan::wrapper::PhysicalDeviceCreationError::PhysicalDeviceCreationError(Type type,
	const std::source_location& location) noexcept :
	m_type(type)
{
}

constexpr nyan::vulkan::wrapper::PhysicalDeviceCreationError::Type nyan::vulkan::wrapper::PhysicalDeviceCreationError::
get_type() const noexcept
{
	return m_type;
}

constexpr const char* nyan::vulkan::wrapper::PhysicalDeviceCreationError::get_message() const noexcept
{
	//return m_message;
	return nullptr;
}

constexpr nyan::vulkan::wrapper::LogicalDeviceCreationError::LogicalDeviceCreationError(Type type,
	const std::source_location& location) noexcept :
	m_type(type)
{
}

constexpr nyan::vulkan::wrapper::LogicalDeviceCreationError::Type nyan::vulkan::wrapper::LogicalDeviceCreationError::
get_type() const noexcept
{
	return m_type;
}

constexpr const char* nyan::vulkan::wrapper::LogicalDeviceCreationError::get_message() const noexcept
{
	//return m_message;
	return nullptr;
}

constexpr nyan::vulkan::wrapper::PhysicalDeviceSelectionError::PhysicalDeviceSelectionError(Type type,
	const std::source_location& location) noexcept :
	m_type(type)
{
}

constexpr nyan::vulkan::wrapper::PhysicalDeviceSelectionError::Type nyan::vulkan::wrapper::PhysicalDeviceSelectionError
::get_type() const noexcept
{
	return m_type;
}

constexpr const char* nyan::vulkan::wrapper::PhysicalDeviceSelectionError::get_message() const noexcept
{
	//return m_message;
	return nullptr;
}
