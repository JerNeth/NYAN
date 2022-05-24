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
	template<typename T, size_t SlotSize>
	class DataManager {
	protected:
		struct Slot {
			std::vector<T> data;
			vulkan::BufferHandle buffer;
			bool dirty;
		};
	public:
		DataManager(vulkan::LogicalDevice& device) :
			r_device(device)
		{

		}

		void upload() {
			for (auto& [bind, slot] : m_slots) {
				if (!slot.dirty)
					continue;
				auto size = sizeof(T) * slot.materialData.size();
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
				m_emptySlots.push_back(bind);
				m_slots.emplace(bind, Slot{ {}, buffer });
				m_slots.find(bind)->second.data.reserve(SlotSize);
			}
			auto empty = m_emptySlots.back();
			auto& slot = m_slots.find(empty)->second;
			auto idx = slot.data.size();
			slot.data.push_back(t);
			if (slot.data.size() > SlotSize)
				m_emptySlots.pop_back();
			slot.dirty = true;
			return Binding{
				.binding{empty},
				.id {static_cast<uint32_t>(idx)},
			};
		}
		void set(const Binding& binding, const T& t) {
			assert(m_slots.find(binding.binding) != m_slots.end());
			auto& slot = m_slots.find(binding.binding)->second;
			assert(slot.data.size() > binding.id);
			slot.data[binding.id] = t;
			slot.dirty = true;
		}
		uint32_t bind_buffer(vulkan::BufferHandle& buffer) {
			return r_device.get_bindless_set().set_storage_buffer(VkDescriptorBufferInfo{ .buffer = buffer->get_handle(), .offset = 0, .range = buffer->get_size() });
		}
		vulkan::BufferHandle create_buffer(size_t size) {
			vulkan::BufferInfo info{
				.size = size * sizeof(T),
				.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
				.offset = 0,
				.memoryUsage = VMA_MEMORY_USAGE_CPU_TO_GPU
			};
			return r_device.create_buffer(info, {});
		}

		vulkan::LogicalDevice& r_device;
		std::unordered_map<uint32_t, Slot> m_slots;
		std::unordered_map<std::string, Binding> m_index;
		std::vector<uint32_t> m_emptySlots;
	};
}
#endif !RDDATAMANAGER_H