#pragma once

#include "VulkanIncludes.h"
#include "VulkanForwards.h"

namespace vulkan {
	class QueryPool : public VulkanObject<VkQueryPool> {
	public:
		QueryPool(LogicalDevice& device, VkQueryPool queryPool, uint32_t maxQueryCount);
		~QueryPool();
		void reset(uint32_t firstQuery, uint32_t queryCount);
		uint32_t& operator++();
		uint32_t operator++(int);
	protected:
		void destroy();
		uint32_t m_usedQueryCount{ 0 };
		uint32_t m_maxQueryCount{ 0 };
	private:
	};
	class TimestampQueryPool : public QueryPool {
	public:
		TimestampQueryPool(LogicalDevice& device, uint32_t maxQueryCount = 64);
		void read_queries();
		void reset();
		const std::vector<uint64_t>& get_results() const noexcept;
	private:
		void create();

		std::vector<uint64_t> m_results;
	};
}
