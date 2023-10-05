#include "VulkanWrapper/QueryPool.hpp"

#include <bit>

#include "VulkanWrapper/LogicalDevice.h"
#include "VulkanWrapper/PhysicalDevice.hpp"

vulkan::QueryPool::QueryPool(LogicalDevice& device, VkQueryPool queryPool, uint32_t maxQueryCount)
	: VulkanObject<VkQueryPool>(device, queryPool),
	m_maxQueryCount(maxQueryCount)
{
}

vulkan::QueryPool::QueryPool(QueryPool&& other) noexcept
	: VulkanObject<VkQueryPool>(std::move(other)),
	m_usedQueryCount(other.m_usedQueryCount),
	m_maxQueryCount(other.m_maxQueryCount)
{
}

vulkan::QueryPool& vulkan::QueryPool::operator=(QueryPool&& other) noexcept
{
	if(this != &other)
	{
		assert(&r_device == &other.r_device);
		m_handle = other.m_handle;
		other.m_handle = VK_NULL_HANDLE;
		m_usedQueryCount = other.m_usedQueryCount;
		m_maxQueryCount = other.m_maxQueryCount;
	}
	return *this;
}

vulkan::QueryPool::~QueryPool()
{
	destroy();
}

void vulkan::QueryPool::reset(uint32_t firstQuery, uint32_t queryCount)
{
	assert(m_handle);
	assert(r_device.get_physical_device().get_vulkan12_features().hostQueryReset);
	r_device.get_device().vkResetQueryPool( m_handle, firstQuery, queryCount);
	m_usedQueryCount = 0;
}

uint32_t& vulkan::QueryPool::operator++()
{
	return ++m_usedQueryCount;
}

uint32_t vulkan::QueryPool::operator++(int)
{
	return m_usedQueryCount++;
}

void vulkan::QueryPool::destroy()
{
	if (m_handle) {
		r_device.get_deletion_queue().queue_query_pool_deletion(m_handle);
		m_handle = VK_NULL_HANDLE;
	}
}

vulkan::TimestampQueryPool::TimestampQueryPool(LogicalDevice& device, uint32_t maxQueryCount)
	: QueryPool(device, VK_NULL_HANDLE, maxQueryCount)
{
	create();
}

void vulkan::TimestampQueryPool::read_queries()
{
	if (!m_usedQueryCount)
		return;
	auto result = r_device.get_device().vkGetQueryPoolResults( m_handle, 0, std::min(m_usedQueryCount, m_maxQueryCount),
		m_results.size() * sizeof(decltype(m_results)::value_type), m_results.data(),
		sizeof(decltype(m_results)::value_type), VK_QUERY_RESULT_64_BIT);
	assert(result == VK_SUCCESS);
}

void vulkan::TimestampQueryPool::reset()
{
	if (!m_usedQueryCount)
		return;
	if (m_usedQueryCount > m_maxQueryCount) {
		destroy();
		m_maxQueryCount = std::bit_width(m_usedQueryCount) << 1;
		create();
	}
	else {
		QueryPool::reset(0, m_usedQueryCount);
	}
}

const std::vector<uint64_t>& vulkan::TimestampQueryPool::get_results() const noexcept
{
	return m_results;
}

void vulkan::TimestampQueryPool::create()
{
	VkQueryPoolCreateInfo createInfo{
		.sType {VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO},
		.pNext {nullptr},
		.flags {},
		.queryType {VK_QUERY_TYPE_TIMESTAMP },
		.queryCount {m_maxQueryCount},
		.pipelineStatistics {}
	};
	auto result = r_device.get_device().vkCreateQueryPool( &createInfo, r_device.get_allocator(), &m_handle);
	m_results.resize(m_maxQueryCount);
	m_usedQueryCount = 0;
	QueryPool::reset(0, m_maxQueryCount);
}

