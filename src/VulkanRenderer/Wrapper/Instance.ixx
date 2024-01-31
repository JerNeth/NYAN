module;

#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <optional>

#include "volk.h"

export module NYANVulkanWrapper:Instance;
import :Error;
import :PhysicalDevice;

export namespace nyan::vulkan::wrapper
{
	class Instance
	{
	public:
		struct ValidationSettings {
			uint32_t enabled : 1 {0};
			uint32_t createCallback : 1 {0};
			uint32_t callBackVerbose : 1 {0};
			uint32_t callBackInfo : 1 {0};
			uint32_t callBackWarning : 1 {0};
			uint32_t callBackError : 1 {0};
			uint32_t callBackGeneral : 1 {0};
			uint32_t callBackValidation : 1 {0};
			uint32_t callBackPerformance : 1 {0};
			uint32_t callBackDeviceAddressBinding : 1 {0};
			uint32_t gpuAssisted : 1 {0};
			uint32_t gpuAssistedReserveBindingSlot : 1 {0};
			uint32_t bestPractices : 1 {0};
			uint32_t debugPrintf : 1 {0};
			uint32_t synchronizationValidation : 1 {0};
			uint32_t disableAll : 1 {0};
			uint32_t disableShaders : 1 {0};
			uint32_t disableThreadSafety : 1 {0};
			uint32_t disableAPIParameters : 1 {0};
			uint32_t disableObjectLifetimes : 1 {0};
			uint32_t disableCoreChecks : 1 {0};
			uint32_t disableUniqueHandles : 1 {0};
			uint32_t disableShaderValidationCache : 1 {0};
		};

		struct ExtensionSettings {
			uint32_t surface : 1 {0};
			uint32_t win32Surface : 1 {0};
			uint32_t xlibSurface : 1 {0};
			uint32_t xcbSurface : 1 {0};
			uint32_t waylandSurface : 1 {0};
			uint32_t androidSurface : 1 {0};
			uint32_t directfbSurface : 1 {0};
			uint32_t acquireXlibSurface : 1 {0};
			uint32_t directModeDisplay : 1 {0}; //Don't actually want that, it's for HMDs, exclusive display access
			uint32_t display : 1 {0}; //Poor support on windows
			uint32_t getDisplayProperties2 : 1 { 0}; //Depends on KHR_DISPLAY
			uint32_t getSurfaceCapabilities2 : 1{ 0};
			uint32_t surfaceMaintenance1 : 1 {0};
			uint32_t getPhysicalDeviceProperties2 : 1 {0}; //1.1 core
			uint32_t swapchainColorSpaceExtension : 1 {0};
			uint32_t validationFeatures : 1 {0};
			uint32_t debugUtils : 1 {0};
			bool operator==(const ExtensionSettings& other) const noexcept = default;
			friend ExtensionSettings operator&(const ExtensionSettings& lhs, const ExtensionSettings& rhs) noexcept
			{
				return ExtensionSettings{
					lhs.surface & rhs.surface,
					lhs.win32Surface & rhs.win32Surface,
					lhs.xlibSurface & rhs.xlibSurface,
					lhs.xcbSurface & rhs.xcbSurface,
					lhs.waylandSurface & rhs.waylandSurface,
					lhs.androidSurface & rhs.androidSurface,
					lhs.directfbSurface & rhs.directfbSurface,
					lhs.acquireXlibSurface & rhs.acquireXlibSurface,
					lhs.directModeDisplay & rhs.directModeDisplay,
					lhs.display & rhs.display,
					lhs.getDisplayProperties2 & rhs.getDisplayProperties2,
					lhs.getSurfaceCapabilities2 & rhs.getSurfaceCapabilities2,
					lhs.surfaceMaintenance1 & rhs.surfaceMaintenance1,
					lhs.getPhysicalDeviceProperties2 & rhs.getPhysicalDeviceProperties2,
					lhs.swapchainColorSpaceExtension & rhs.swapchainColorSpaceExtension,
					lhs.validationFeatures & rhs.validationFeatures,
					lhs.debugUtils & rhs.debugUtils
				};
			}
			friend ExtensionSettings operator|(const ExtensionSettings& lhs, const ExtensionSettings& rhs) noexcept
			{
				return ExtensionSettings{
					lhs.surface | rhs.surface,
					lhs.win32Surface | rhs.win32Surface,
					lhs.xlibSurface | rhs.xlibSurface,
					lhs.xcbSurface | rhs.xcbSurface,
					lhs.waylandSurface | rhs.waylandSurface,
					lhs.androidSurface | rhs.androidSurface,
					lhs.directfbSurface | rhs.directfbSurface,
					lhs.acquireXlibSurface | rhs.acquireXlibSurface,
					lhs.directModeDisplay | rhs.directModeDisplay,
					lhs.display | rhs.display,
					lhs.getDisplayProperties2 | rhs.getDisplayProperties2,
					lhs.getSurfaceCapabilities2 | rhs.getSurfaceCapabilities2,
					lhs.surfaceMaintenance1 | rhs.surfaceMaintenance1,
					lhs.getPhysicalDeviceProperties2 | rhs.getPhysicalDeviceProperties2,
					lhs.swapchainColorSpaceExtension | rhs.swapchainColorSpaceExtension,
					lhs.validationFeatures | rhs.validationFeatures,
					lhs.debugUtils | rhs.debugUtils
				};
			}
		};
		~Instance() noexcept;
		Instance(Instance&) = delete;
		Instance& operator=(Instance&) = delete;
		Instance(Instance&& other) noexcept;
		Instance& operator=(Instance&& other) noexcept;
		[[nodiscard]] const std::vector<PhysicalDevice>& get_physical_devices() const noexcept;

		[[nodiscard]] std::expected<PhysicalDevice, PhysicalDeviceSelectionError> select_physical_device(std::optional<uint64_t> deviceId, const PhysicalDevice::Extensions& requiredExtensions) const noexcept;

		[[nodiscard]] static std::expected<Instance, InstanceCreationError> create(const ValidationSettings& validationSettings,
		                                                             const ExtensionSettings& requiredExtensions, const ExtensionSettings& optionalExtension,
		                                                             std::string_view applicationName = "", std::string_view engineName = "",
		                                                             uint32_t applicationVersion = 0, uint32_t engineVersion = 0) noexcept;

		[[nodiscard]] VkInstance get_handle() const noexcept;
	private:
		Instance(VkInstance instance, VkDebugUtilsMessengerEXT debugUtilsMessenger, VkAllocationCallbacks* allocator, 
			std::vector<PhysicalDevice> physicalDevices, const ValidationSettings& validationSettings, ExtensionSettings enabledExtensions) noexcept;

		VkInstance m_handle{ VK_NULL_HANDLE };
		VkDebugUtilsMessengerEXT m_debugUtilsMessenger{ VK_NULL_HANDLE };
		VkAllocationCallbacks* m_allocator{ nullptr };

		std::vector<PhysicalDevice> m_physicalDevices{};

		ValidationSettings m_validationSettings;
		ExtensionSettings m_enabledExtensions;
	};
}
