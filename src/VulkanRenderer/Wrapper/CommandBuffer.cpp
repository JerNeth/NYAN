// ReSharper disable CppMemberFunctionMayBeConst
// Functions are technically const but semantically they modify something
module;

#include <cassert>
#include <utility>

#include "volk.h"

module NYANVulkanWrapper;

using namespace nyan::vulkan::wrapper;

CommandBuffer::CommandBuffer(CommandBuffer&& other) noexcept :
	Object(other.r_device, other.m_handle),
	m_state(other.m_state)
{
	other.m_handle = VK_NULL_HANDLE;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other) noexcept
{
	if(this != std::addressof(other))
	{
		assert(std::addressof(r_device) == std::addressof(other.r_device));
		std::swap(m_handle, other.m_handle);
		std::swap(m_state, other.m_state);
	}
	return *this;
}

std::expected<void, Error> CommandBuffer::begin(const bool oneTimeSubmit) noexcept
{
	assert(m_state == State::Initial);
	if(m_state != State::Initial) //We assume that we only use completely reset commandPools
		return std::unexpected{ VK_ERROR_UNKNOWN };

	const VkCommandBufferBeginInfo beginInfo{
		.sType {VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO},
		.pNext {nullptr}, //Only currently supported is DeviceGroups, which are kinda deprecated anyways
		.flags {oneTimeSubmit ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT : static_cast<VkCommandBufferUsageFlags>(0)},
		.pInheritanceInfo {nullptr} //Secondary command buffers are not that useful so currently skip support
	};
	if (const auto result = r_device.vkBeginCommandBuffer(m_handle, &beginInfo); result != VK_SUCCESS)
		return std::unexpected{ result };
	return {};
}

std::expected<void, Error> CommandBuffer::end() noexcept
{
	assert(m_state == State::Recording);
	if (m_state != State::Recording)
		return std::unexpected{ VK_ERROR_UNKNOWN };

	if (const auto result = r_device.vkEndCommandBuffer(m_handle); result != VK_SUCCESS)
		return std::unexpected{ result };
	return {};
}

//void CommandBuffer::begin_rendering(const VkRenderingInfo& info) noexcept
//{
//	r_device.vkCmdBeginRendering(m_handle, &info);
//}
//
//void CommandBuffer::end_rendering() noexcept
//{
//	r_device.vkCmdEndRendering(m_handle);
//}

void CommandBuffer::begin_region(const char* name, const float* color) noexcept
{
	assert(vkCmdBeginDebugUtilsLabelEXT);
	VkDebugUtilsLabelEXT label{
		.sType {VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT},
		.pNext {nullptr},
		.pLabelName {name}
	};
	if (color) {
		for (uint32_t i = 0; i < 4; i++)
			label.color[i] = color[i];
	}
	else {
		for (uint32_t i = 0; i < 4; i++)
			label.color[i] = 1.0f;
	}
	vkCmdBeginDebugUtilsLabelEXT(m_handle, &label);
}

void CommandBuffer::end_region() noexcept
{
	assert(vkCmdEndDebugUtilsLabelEXT);
	vkCmdEndDebugUtilsLabelEXT(m_handle);
}

CommandBuffer::~CommandBuffer() noexcept
{

}

Queue::Type CommandBuffer::get_type() const noexcept
{
	return m_type;
}

CommandBuffer::CommandBuffer(const LogicalDeviceWrapper& device,
                             const VkCommandBuffer handle, 
                             const Queue::Type type) noexcept :
	Object(device, handle),
	m_state(State::Initial),
	m_type(type)
{
}
