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
			vulkan::BufferHandle buffer;
			uint32_t slotCapacity;
			bool dirty;
		};
	public:
		DataManager(vulkan::LogicalDevice& device, VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, uint32_t initialSize = 8192) :
			r_device(device),
			m_usage(usage)
		{
			auto buffer = create_buffer(initialSize);
			m_slot = std::make_unique<Slot>(Slot{ bind_buffer(buffer), {}, buffer, initialSize, true });
			m_slot->data.reserve(initialSize);
		}

		void upload() {
			if (!m_slot->dirty)
				return;
			if (m_slot->slotCapacity < m_slot->data.size()) {
				m_slot->buffer = create_buffer(m_slot->data.capacity());
				rebind_buffer(m_slot->buffer);
				m_slot->slotCapacity = m_slot->data.capacity();
			}
			auto size = sizeof(T) * m_slot->data.size();
			auto* map = m_slot->buffer->map_data();
			std::memcpy(map, m_slot->data.data(), size);
			m_slot->buffer->flush(0, size);
			m_slot->dirty = false;

		}

		uint32_t get_binding() const {
			return m_slot->binding;
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
		uint32_t bind_buffer(vulkan::BufferHandle& buffer) {
			return r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
		}
		void rebind_buffer(vulkan::BufferHandle& buffer) {
			r_device.get_bindless_set().set_storage_buffer(m_slot->binding, VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
		}
		vulkan::BufferHandle create_buffer(size_t size) {
			vulkan::BufferInfo info{
				.size = size * sizeof(T),
				.usage = m_usage,
				.offset = 0,
				.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
			};
			return r_device.create_buffer(info, {});
		}

		vulkan::LogicalDevice& r_device;
		VkBufferUsageFlags m_usage;
		std::unique_ptr<Slot> m_slot;
	};
}
#endif !RDDATAMANAGER_H