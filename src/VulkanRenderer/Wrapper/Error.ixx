module;

#include <array>
#include <source_location>
#include <string_view>

#include "magic_enum_all.hpp"

#include "volk.h"

export module NYANVulkanWrapper:Error;

export namespace nyan::vulkan::wrapper
{
	class Error
	{
	public:
		enum class Type : int32_t 
		{
			Success  = VK_SUCCESS ,
			NotReady  = VK_NOT_READY ,
			Timeout  = VK_TIMEOUT ,
			EventSet  = VK_EVENT_SET ,
			EventReset  = VK_EVENT_RESET ,
			Incomplete  = VK_INCOMPLETE ,
			ErrorOutOfHostMemory  = VK_ERROR_OUT_OF_HOST_MEMORY ,
			ErrorOutOfDeviceMemory  = VK_ERROR_OUT_OF_DEVICE_MEMORY ,
			ErrorInitializationFailed  = VK_ERROR_INITIALIZATION_FAILED ,
			ErrorDeviceLost  = VK_ERROR_DEVICE_LOST ,
			ErrorMemoryMapFailed  = VK_ERROR_MEMORY_MAP_FAILED ,
			ErrorLayerNotPresent  = VK_ERROR_LAYER_NOT_PRESENT ,
			ErrorExtensionNotPresent  = VK_ERROR_EXTENSION_NOT_PRESENT ,
			ErrorFeatureNotPresent  = VK_ERROR_FEATURE_NOT_PRESENT ,
			ErrorIncompatibleDriver  = VK_ERROR_INCOMPATIBLE_DRIVER ,
			ErrorTooManyObjects  = VK_ERROR_TOO_MANY_OBJECTS ,
			ErrorFormatNotSupported  = VK_ERROR_FORMAT_NOT_SUPPORTED ,
			ErrorErrorFragmentedPool  = VK_ERROR_FRAGMENTED_POOL ,
			ErrorUnknown  = VK_ERROR_UNKNOWN ,
			ErrorOutOfPoolMemory  = VK_ERROR_OUT_OF_POOL_MEMORY ,
			ErrorInvalidExternalHandle  = VK_ERROR_INVALID_EXTERNAL_HANDLE ,
			ErrorFragmentation  = VK_ERROR_FRAGMENTATION ,
			ErrorInvalidOpaqueCaptureAddress  = VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS ,
			PipelineCompileRequired  = VK_PIPELINE_COMPILE_REQUIRED ,
			ErrorSurfaceLost  = VK_ERROR_SURFACE_LOST_KHR ,
			ErrorNativeWindowInUse = VK_ERROR_NATIVE_WINDOW_IN_USE_KHR ,
			Suboptimal  = VK_SUBOPTIMAL_KHR ,
			ErrorOutOfDate = VK_ERROR_OUT_OF_DATE_KHR ,
			ErrorIncompatibleDisplay = VK_ERROR_INCOMPATIBLE_DISPLAY_KHR ,
			ErrorValidationFailed = VK_ERROR_VALIDATION_FAILED_EXT ,
			ErrorInvalidShader = VK_ERROR_INVALID_SHADER_NV ,
			ErrorImageUsageNotSupported = VK_ERROR_IMAGE_USAGE_NOT_SUPPORTED_KHR ,
			ErrorVideoPictureLayoutNotSupported = VK_ERROR_VIDEO_PICTURE_LAYOUT_NOT_SUPPORTED_KHR ,
			ErrorVideoProfileOperationNotSupported = VK_ERROR_VIDEO_PROFILE_OPERATION_NOT_SUPPORTED_KHR ,
			ErrorVideoProfileFormatNotSupported = VK_ERROR_VIDEO_PROFILE_FORMAT_NOT_SUPPORTED_KHR ,
			ErrorVideoProfileCodecNotSupported = VK_ERROR_VIDEO_PROFILE_CODEC_NOT_SUPPORTED_KHR ,
			ErrorVideoStdVersionNotSupported = VK_ERROR_VIDEO_STD_VERSION_NOT_SUPPORTED_KHR ,
			ErrorInvalidDrmFormatModifierPlaneLayout = VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT ,
			ErrorNotPermited = VK_ERROR_NOT_PERMITTED_KHR ,
			ErrorFullScreenExclusiveModeLost = VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT ,
			ThreadIdle  = VK_THREAD_IDLE_KHR ,
			ThreadDone = VK_THREAD_DONE_KHR ,
			OperationDeferred  = VK_OPERATION_DEFERRED_KHR ,
			OperationNotDeferred  = VK_OPERATION_NOT_DEFERRED_KHR ,
			ErrorInvalidVideoStdParameters  = VK_ERROR_INVALID_VIDEO_STD_PARAMETERS_KHR ,
			ErrorCompressionExhausted  = VK_ERROR_COMPRESSION_EXHAUSTED_EXT ,
			ErrorIncompatibleShaderBinary  = VK_ERROR_INCOMPATIBLE_SHADER_BINARY_EXT ,
		};
	public:
		constexpr Error(VkResult result, const std::source_location& location = std::source_location::current()) noexcept;
		constexpr explicit operator Type() const noexcept;
		[[nodiscard]] constexpr Type get_result() const noexcept;
		[[nodiscard]] constexpr const char* get_message() const noexcept;
	private:
		Type m_result{ Type::Success };
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
		[[nodiscard]] const char* get_message() const noexcept;
	private:
		Type m_type{ };
		//const char* m_message{ "" };
	};

}

constexpr nyan::vulkan::wrapper::Error::Error(const VkResult result, const std::source_location& location) noexcept :
	m_result(static_cast<Type>(result))
	//m_message("")
{
}


constexpr nyan::vulkan::wrapper::Error::operator nyan::vulkan::wrapper::Error::Type() const noexcept
{
	return get_result();
}

constexpr nyan::vulkan::wrapper::Error::Type nyan::vulkan::wrapper::Error::get_result() const noexcept
{
	return m_result;
}
constexpr const char* nyan::vulkan::wrapper::Error::get_message() const noexcept
{
	//magic_enum::enum_names(m_result);
	return "";
	//return magic_enum::enum_name(m_result).data();
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

const char* nyan::vulkan::wrapper::PhysicalDeviceSelectionError::get_message() const noexcept
{
	//return m_message;
	//return magic_enum::enum_names<nyan::vulkan::wrapper::PhysicalDeviceSelectionError::Type>()[static_cast<std::underlying_type<nyan::vulkan::wrapper::PhysicalDeviceSelectionError::Type>::type>(m_type)].data();
	//auto a = magic_enum::enum_name(m_type);
	return magic_enum::enum_name(m_type).data();
}
