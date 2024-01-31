module;

#include <cassert>
#include <string_view>

#include "volk.h"

module NYANVulkanWrapper;
import NYANLog;

using namespace nyan::vulkan::wrapper;

Queue::Queue(const LogicalDeviceWrapper& device, VkQueue handle, Type type, uint32_t queueFamilyIndex, float priority) noexcept :
	Object(device, handle),
	m_type(type),
	m_queueFamilyIndex(queueFamilyIndex),
	m_priority(priority)
{
}

std::expected<void, Error> Queue::wait_idle() const noexcept
{

	if(const auto result = r_device.vkQueueWaitIdle(m_handle); result != VK_SUCCESS)
	{
		return std::unexpected{ result };
	}

	return {};
}

std::expected<void, Error> Queue::submit() const noexcept
{
	assert(false);
	return std::unexpected{ VK_ERROR_DEVICE_LOST };
}

std::expected<void, Error> Queue::submit2() const noexcept
{
	assert(false);
	return std::unexpected{ VK_ERROR_DEVICE_LOST };
}

std::expected<void, Error> Queue::present() const noexcept
{
	assert(false);
	assert(m_presentCapable);
	return std::unexpected{ VK_ERROR_DEVICE_LOST };
}

void Queue::insert_debug_utils_label(std::string_view labelName, const std::array<float, 4> color) const noexcept
{
	//if (!r_device.get_enabled_extensions().debugUtils)
	//	return;
	const VkDebugUtilsLabelEXT label {
		.sType { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {labelName.data()},
		.color {color[0], color[1], color[2], color[3]}
	};
	vkQueueInsertDebugUtilsLabelEXT(m_handle, &label);
}

void Queue::begin_debug_utils_label(std::string_view labelName, std::array<float, 4> color) const noexcept
{
	//if (!r_device.get_enabled_extensions().debugUtils)
	//	return;
	const VkDebugUtilsLabelEXT label{
		.sType { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {labelName.data()},
		.color {color[0], color[1], color[2], color[3]}
	};
	vkQueueBeginDebugUtilsLabelEXT(m_handle, &label);
}

void Queue::end_debug_utils_label() const noexcept
{
	vkQueueEndDebugUtilsLabelEXT(m_handle);
}

Queue::Type Queue::get_type() const noexcept
{
	return m_type;
}

uint32_t Queue::get_queue_family_index() const noexcept
{
	return m_queueFamilyIndex;
}

float Queue::get_priority() const noexcept
{
	return m_priority;
}

bool Queue::is_present_capable() const noexcept
{
	return m_presentCapable;
}
