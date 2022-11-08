#pragma once
#ifndef RDDATAMANAGER_H
#define RDDATAMANAGER_H
#include "LogicalDevice.h"


namespace nyan {
	template<typename T>
	class DataManager {
	protected:
		struct Slot {
			uint32_t binding;
			std::vector<T> data;
			vulkan::BufferHandle deviceBuffer;
			vulkan::BufferHandle stagingBuffer;
			uint32_t slotCapacity;
			bool dirty;
		};
	public:
		DataManager(vulkan::LogicalDevice& device, VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, uint32_t initialSize = 8192) :
			r_device(device),
			m_usage(usage)
		{
			m_ssbo = (m_usage & VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) != VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

			auto stagingBuffer = create_buffer(initialSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
			auto deviceBuffer = create_buffer(initialSize, m_usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VMA_MEMORY_USAGE_GPU_ONLY);
			m_slot = std::make_unique<Slot>(Slot{
				.binding{ bind_buffer(deviceBuffer) },
				.data{},
				.deviceBuffer{deviceBuffer},
				.stagingBuffer{stagingBuffer},
				.slotCapacity{initialSize},
				.dirty{true }
			});
			m_slot->data.reserve(initialSize);
		}

		bool upload(vulkan::CommandBuffer& cmd);

		uint32_t get_binding() const {
			return m_slot->binding;
		}
		size_t slot_count() const {
			return m_slot->data.size();
		}
	protected:
		uint32_t add(const T& t) {
			auto idx = m_slot->data.size();
			m_slot->data.push_back(t);
			m_slot->dirty = true;
			return static_cast<uint32_t>(idx);
		}
		void set(const uint32_t& id, const T& t) {
			assert(m_slot->data.size() > id);
			m_slot->data[id] = t;
			m_slot->dirty = true;
		}
		T& get(const uint32_t& id) {
			assert(m_slot->data.size() > id);
			m_slot->dirty = true;
			return m_slot->data[id];
		}
		const T& get(const uint32_t& id) const {
			assert(m_slot->data.size() > id);
			return m_slot->data[id];
		}
		uint32_t bind_buffer(vulkan::BufferHandle& buffer);
		void rebind_buffer(vulkan::BufferHandle& buffer);
		vulkan::BufferHandle create_buffer(size_t size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU);

		vulkan::LogicalDevice& r_device;
		VkBufferUsageFlags m_usage;
		std::unique_ptr<Slot> m_slot;
		bool m_ssbo;
	};
}
#endif !RDDATAMANAGER_H