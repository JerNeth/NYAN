#pragma once
#ifndef RDDATAMANAGER_H
#define RDDATAMANAGER_H
#include "LogicalDevice.h"


namespace nyan {

	struct Binding {
		union {
			struct {
				uint32_t binding;
				uint32_t id;
			};
			uint64_t data;
		};
		template<std::size_t Index>
		auto get() const
		{
			static_assert(Index < 2,
				"Index out of bounds for Custom::Binding");
			if constexpr (Index == 0) return id;
			if constexpr (Index == 1) return binding;
		}
	};
	template<typename T, typename Binding, size_t SlotSize>
	class DataManager {
	protected:
		struct Slot {
			uint32_t binding;
			std::vector<T> data;
			vulkan::BufferHandle buffer;
			bool dirty;
		};
	public:
		DataManager(vulkan::LogicalDevice& device, VkBufferUsageFlags usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) :
			r_device(device),
			m_usage(usage)
		{

		}

		void upload() {
			for (auto& slot : m_slots) {
				if (!slot.dirty)
					continue;
				auto size = sizeof(T) * slot.data.size();
				auto* map = slot.buffer->map_data();
				std::memcpy(map, slot.data.data(), size);
				slot.buffer->flush(0, size);
				slot.dirty = false;
			}
		}

	protected:
		Binding add(const T& t) {
			if (m_emptySlots.empty()) {
				auto buffer = create_buffer(SlotSize);
				auto bind = bind_buffer(buffer);
				m_emptySlots.push_back(m_slots.size());
				m_slots.push_back(Slot{ bind,{}, buffer });
				m_slots.back().data.reserve(SlotSize);
			}
			auto empty = m_emptySlots.back();
			auto& slot = m_slots[empty];
			auto idx = slot.data.size();
			slot.data.push_back(t);
			if (slot.data.size() > SlotSize)
				m_emptySlots.pop_back();
			slot.dirty = true;
			return Binding{
				.binding{slot.binding},
				.id {static_cast<uint32_t>(idx)},
			};
		}
		void set(const Binding& binding, const T& t) {
			auto slot = std::find_if(m_slots.begin(), m_slots.end(), [binding](const auto& slot) {return slot.binding == binding.binding; });
			assert(slot != m_slots.end());
			assert(slot->data.size() > binding.id);
			slot->data[binding.id] = t;
			slot->dirty = true;
		}
		T& get(const Binding& binding) {
			auto slot = std::find_if(m_slots.begin(), m_slots.end(), [binding](const auto& slot) {return slot.binding == binding.binding; });
			assert(slot != m_slots.end());
			assert(slot->data.size() > binding.id);
			slot->dirty = true;
			return slot->data[binding.id];
		}
		const T& get(const Binding& binding) const {
			auto slot = std::find_if(m_slots.begin(), m_slots.end(), [binding](const auto& slot) {return slot.binding == binding.binding; });
			assert(slot != m_slots.end());
			assert(slot->data.size() > binding.id);
			return slot->data[binding.id];
		}
		uint32_t bind_buffer(vulkan::BufferHandle& buffer) {
			return r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
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
		std::vector<Slot> m_slots;
		std::unordered_map<std::string, Binding> m_index;
		std::vector<uint32_t> m_emptySlots;
	};
}
#endif !RDDATAMANAGER_H