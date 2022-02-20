#pragma once
#ifndef VKACCELERATIONSTRUCTURE_H
#define VKACCELERATIONSTRUCTURE_H
#include <Util>

namespace vulkan {
	class LogicalDevice;
	class AccelerationStructure : public Utility::UIDC {
	public:
		AccelerationStructure(LogicalDevice& device);
		AccelerationStructure(AccelerationStructure& ) = delete;
		AccelerationStructure(AccelerationStructure&&) = delete;
		AccelerationStructure& operator=(AccelerationStructure&) = delete;
		AccelerationStructure& operator=(AccelerationStructure&&) = delete;
		VkAccelerationStructureKHR get_handle() const noexcept;
		bool is_compactable() const noexcept;
	private:
		LogicalDevice& r_device;
		VkAccelerationStructureKHR m_handle;
		bool m_isCompactable = false;
	};
}

#endif !VKACCELERATIONSTRUCTURE_H