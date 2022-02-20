#include "..\..\include\VulkanWrapper\AccelerationStructure.h"
#include "AccelerationStructure.h"

vulkan::AccelerationStructure::AccelerationStructure(LogicalDevice& device) :
	r_device(device)
{
}

VkAccelerationStructureKHR vulkan::AccelerationStructure::get_handle() const noexcept
{
	return m_handle;
}

bool vulkan::AccelerationStructure::is_compactable() const noexcept
{
	return m_isCompactable;
}
