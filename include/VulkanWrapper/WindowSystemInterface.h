#ifndef VKWINDOWSYSTEMINTERFACE_H
#define VKWINDOWSYSTEMINTERFACE_H
#include "VulkanIncludes.h"
namespace Vulkan {
	class LogicalDevice;
	class WindowSystemInterface {
	public:
		WindowSystemInterface(const char** extensions, uint32_t extensionCount, std::string applicationName = "", std::string engineName = "");
	private:
	};
}

#endif //VKWINDOWSYSTEMINTERFACE_H!