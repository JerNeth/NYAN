#include "QueryPool.hpp"
#include "LogicalDevice.h"
#include "Instance.h"
#include <bit>

vulkan::QueryPool::QueryPool(LogicalDevice& device, VkQueryPool queryPool, uint32_t maxQueryCount)
	: VulkanObject<VkQueryPool>(device, queryPool),
	m_maxQueryCount(maxQueryCount)
{
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
	if (m_handle)
		r_device.get_device().vkDestroyQueryPool(m_handle, r_device.get_allocator());
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
	r_device.get_device().vkCreateQueryPool( &createInfo, r_device.get_allocator(), &m_handle);
	m_results.resize(m_maxQueryCount);
	m_usedQueryCount = 0;
	QueryPool::reset(0, m_maxQueryCount);
}

