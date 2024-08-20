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
	m_priority(priority),
	m_minImageTransferGranularity(0, 0, 0),
	m_presentId(0)
{
	assert(m_queueFamilyIndex != invalidQueueFamilyIndex);
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
	assert(false && "TODO");
	return std::unexpected{ VK_ERROR_DEVICE_LOST };
}

std::expected<void, Error> nyan::vulkan::wrapper::Queue::submit2(CommandBuffer& cmd, Fence* fence) const noexcept
{
	VkLatencySubmissionPresentIdNV presentId{
		.sType {VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV},
		.pNext {nullptr},
		.presentID {m_presentId},
	};
	assert(CommandBufferAccessor::state(cmd) == CommandBuffer::State::Executable);
	CommandBufferAccessor::state(cmd) = CommandBuffer::State::Pending;

	VkCommandBufferSubmitInfo cmdInfo{
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO},
		.pNext {nullptr},
		.commandBuffer {cmd.get_handle()},
		.deviceMask {0}
	};

	VkSubmitInfo2 submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO_2},
		.pNext {nullptr},
		.flags {0},
		.waitSemaphoreInfoCount {0},
		.pWaitSemaphoreInfos {nullptr},
		.commandBufferInfoCount {1},
		.pCommandBufferInfos {&cmdInfo},
		.signalSemaphoreInfoCount {0},
		.pSignalSemaphoreInfos {nullptr}
	};
	if (m_enableLowLatencyNV)
		submitInfo.pNext = &presentId;
	VkFence fenceHandle{ VK_NULL_HANDLE };
	if (fence) {
		assert(!fence->is_signaled());
		if (fence->is_signaled())
			return std::unexpected{ VK_ERROR_UNKNOWN };
		fenceHandle = fence->get_handle();
		fence->set_signaled();
	}

	if (auto result = r_device.vkQueueSubmit2(m_handle, 1, &submitInfo, fenceHandle); result != VK_SUCCESS)
		return std::unexpected{ result };
	
	return {};
}

std::expected<void, Error> Queue::present() const noexcept
{
	assert(false && "TODO");
	assert(m_presentCapable);
	return std::unexpected{ VK_ERROR_DEVICE_LOST };
}

void Queue::insert_debug_utils_label(const char* labelName, const std::array<float, 4> color) const noexcept
{
	//if (!r_device.get_enabled_extensions().debugUtils)
	//	return;
	const VkDebugUtilsLabelEXT label {
		.sType { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {labelName},
		.color {color[0], color[1], color[2], color[3]}
	};
	vkQueueInsertDebugUtilsLabelEXT(m_handle, &label);
}

void Queue::begin_debug_utils_label(const char* labelName, const std::array<float, 4> color) const noexcept
{
	//if (!r_device.get_enabled_extensions().debugUtils)
	//	return;
	const VkDebugUtilsLabelEXT label{
		.sType { VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {labelName},
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

bool nyan::vulkan::wrapper::Queue::is_sparse_capable() const noexcept
{
	return m_sparseCapable;
}

const std::array<uint32_t, 3>& nyan::vulkan::wrapper::Queue::transfer_granularity() const noexcept
{
	assert(false && "TODO");
	return m_minImageTransferGranularity;
}
