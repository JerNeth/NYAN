module;

//#include <cassert>
//#include <expected>
//#include <string_view>

#include "volk.h"

module NYANVulkan;
import std;

import NYANLog;

using namespace nyan::vulkan;

Queue::Queue(Queue&& other) noexcept :
	Object(*other.ptr_device, std::exchange(other.m_handle, VK_NULL_HANDLE)),
	m_type(other.m_type),
	m_queueFamilyIndex(other.m_queueFamilyIndex),
	m_priority(other.m_priority),
	m_minImageTransferGranularity(other.m_minImageTransferGranularity),
	m_presentId(other.m_presentId)
{
}

Queue& Queue::operator=(Queue&& other) noexcept
{
	if (std::addressof(other) != this) {
		std::swap(ptr_device, other.ptr_device);
		std::swap(m_handle, other.m_handle);
		std::swap(m_type, other.m_type);
		std::swap(m_queueFamilyIndex, other.m_queueFamilyIndex);
		std::swap(m_priority, other.m_priority);
		std::swap(m_minImageTransferGranularity, other.m_minImageTransferGranularity);
		std::swap(m_presentId, other.m_presentId);
	}
	return *this;
}

Queue::Queue(const LogicalDeviceWrapper& device, VkQueue handle, Type type, Queue::FamilyIndex queueFamilyIndex, float priority, std::array<uint32_t, 3> transferGranularity) noexcept :
	Object(device, handle),
	m_type(type),
	m_queueFamilyIndex(queueFamilyIndex),
	m_priority(priority),
	m_minImageTransferGranularity(transferGranularity),
	m_presentId(0)
{
	::assert(m_queueFamilyIndex.value != Queue::FamilyIndex::invalid);
}

std::expected<void, Error> Queue::wait_idle() const noexcept
{

	if(const auto result = ptr_device->vkQueueWaitIdle(m_handle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit(CommandBuffer& cmd, BinarySemaphore& waitSemaphore, BinarySemaphore& signalSemaphore) const noexcept
{
	VkPipelineStageFlags flags{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO},
		.pNext {nullptr},
		.waitSemaphoreCount {0},
		.pWaitSemaphores {nullptr},
		.pWaitDstStageMask { &flags},
		.commandBufferCount { 1},
		.pCommandBuffers {&cmd.get_handle()},
		.signalSemaphoreCount {0},
		.pSignalSemaphores{nullptr}
	};
	if (!waitSemaphore.signaled()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	waitSemaphore.m_timeline++;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore.get_handle();

	if (!signalSemaphore.waited()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	signalSemaphore.m_timeline++;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore.get_handle();
	

	if (auto result = ptr_device->vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit(CommandBuffer& cmd) const noexcept
{
	VkSubmitInfo submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO},
		.pNext {nullptr},
		.waitSemaphoreCount {0},
		.pWaitSemaphores {nullptr},
		.pWaitDstStageMask { nullptr},
		.commandBufferCount { 1},
		.pCommandBuffers {&cmd.get_handle()},
		.signalSemaphoreCount {0},
		.pSignalSemaphores{nullptr}
	};

	if (auto result = ptr_device->vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit_signal(CommandBuffer& cmd, BinarySemaphore& signalSemaphore) const noexcept
{
	VkSubmitInfo submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO},
		.pNext {nullptr},
		.waitSemaphoreCount {0},
		.pWaitSemaphores {nullptr},
		.pWaitDstStageMask { nullptr},
		.commandBufferCount { 1},
		.pCommandBuffers {&cmd.get_handle()},
		.signalSemaphoreCount {0},
		.pSignalSemaphores{nullptr}
	};

	if (!signalSemaphore.waited()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	signalSemaphore.m_timeline++;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &signalSemaphore.get_handle();


	if (auto result = ptr_device->vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit_wait(CommandBuffer& cmd, BinarySemaphore& waitSemaphore) const noexcept
{
	VkPipelineStageFlags flags{ VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO},
		.pNext {nullptr},
		.waitSemaphoreCount {0},
		.pWaitSemaphores {nullptr},
		.pWaitDstStageMask { &flags},
		.commandBufferCount { 1},
		.pCommandBuffers {&cmd.get_handle()},
		.signalSemaphoreCount {0},
		.pSignalSemaphores{nullptr}
	};
	if (!waitSemaphore.signaled()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	waitSemaphore.m_timeline++;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &waitSemaphore.get_handle();

	if (auto result = ptr_device->vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit(BinarySemaphore& semaphore) const noexcept
{
	if (!semaphore.signaled()) [[unlikely]] {
		::assert(false);
		return std::unexpected{ VK_ERROR_UNKNOWN };
	}
	semaphore.m_timeline++;

	VkPipelineStageFlags flags{ VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };
	VkSubmitInfo submitInfo{
		.sType {VK_STRUCTURE_TYPE_SUBMIT_INFO},
		.pNext {nullptr},
		.waitSemaphoreCount {1},
		.pWaitSemaphores {&semaphore.get_handle()},
		.pWaitDstStageMask { &flags},
		.commandBufferCount { 0},
		.pCommandBuffers {nullptr},
		.signalSemaphoreCount {0},
		.pSignalSemaphores{nullptr}
	};

	if (auto result = ptr_device->vkQueueSubmit(m_handle, 1, &submitInfo, VK_NULL_HANDLE); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

std::expected<void, Error> Queue::submit2(CommandBuffer& cmd, Fence* fence) const noexcept
{
	VkLatencySubmissionPresentIdNV presentId{
		.sType {VK_STRUCTURE_TYPE_LATENCY_SUBMISSION_PRESENT_ID_NV},
		.pNext {nullptr},
		.presentID {m_presentId},
	};
	//assert(CommandBufferAccessor::state(cmd) == CommandBuffer::State::Executable);
	//CommandBufferAccessor::state(cmd) = CommandBuffer::State::Pending;

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
		if (fence->is_signaled()) [[unlikely]] {
			::assert(false);
			return std::unexpected{ VK_ERROR_UNKNOWN };
		}
		fenceHandle = fence->get_handle();
		fence->set_signaled();
	}

	if (auto result = ptr_device->vkQueueSubmit2(m_handle, 1, &submitInfo, fenceHandle); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };
	
	return {};
}

std::expected<void, Error> Queue::present(Swapchain& swapchain, BinarySemaphore* semaphore) const noexcept
{
	//assert(m_presentCapable);
	//if(!m_presentCapable)
	//	return std::unexpected{ VK_ERROR_DEVICE_LOST };
	if (semaphore) {
		if (!semaphore->signaled()) [[unlikely]] {
			::assert(false);
			return std::unexpected{ VK_ERROR_DEVICE_LOST };
		}
		semaphore->m_timeline++;
	}

	uint32_t index = swapchain.get_index();

	VkPresentInfoKHR presentInfo{
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = semaphore != nullptr ? 1u : 0u,
		.pWaitSemaphores = semaphore != nullptr? &semaphore->get_handle() : nullptr,
		.swapchainCount = 1,
		.pSwapchains = &swapchain.get_handle(),
		.pImageIndices = &index,
		.pResults = NULL
	};

	if (auto result = ptr_device->vkQueuePresentKHR(m_handle, &presentInfo); result != VK_SUCCESS) [[unlikely]]
		return std::unexpected{ result };

	return {};
}

void Queue::insert_debug_utils_label(const char* labelName, const std::array<float, 4> color) const noexcept
{
	//if (!ptr_device->get_enabled_extensions().debugUtils)
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
	//if (!ptr_device->get_enabled_extensions().debugUtils)
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

Queue::FamilyIndex Queue::get_queue_family_index() const noexcept
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

bool Queue::is_sparse_capable() const noexcept
{
	return m_sparseCapable;
}

const std::array<uint32_t, 3>& Queue::transfer_granularity() const noexcept
{
	return m_minImageTransferGranularity;
}

//std::span<const CommandPool> Queue::get_command_pools() const noexcept
//{
//	return {m_commandPools.data(), m_commandPools.size() };
//}
//
//std::span<CommandPool> Queue::get_command_pools() noexcept
//{
//	return { m_commandPools.data(), m_commandPools.size() };
//}
