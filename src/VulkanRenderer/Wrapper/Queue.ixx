module;

#include <expected>
#include <string_view>
#include <array>
#include <vector>

#include "magic_enum.hpp"
#include "magic_enum_containers.hpp"

#include "volk.h"
#include "vk_mem_alloc.h"

export module NYANVulkanWrapper:Queue;
import :Error;
import :Object;

export namespace nyan::vulkan::wrapper
{
	class Queue : public Object<VkQueue>
	{
	public:
		enum class Type : uint32_t
		{
			Graphics,
			Compute,
			Transfer,
			Encode,
			Decode
		};
	public:
		Queue(const LogicalDeviceWrapper& device, VkQueue handle, Type type, uint32_t queueFamilyIndex, float priority) noexcept;

		[[nodiscard]] std::expected<void, Error> wait_idle() const noexcept;
		[[nodiscard]] std::expected<void, Error> submit() const noexcept;
		[[nodiscard]] std::expected<void, Error> submit2() const noexcept;
		[[nodiscard]] std::expected<void, Error> present() const noexcept;

		void insert_debug_utils_label(std::string_view labelName, std::array<float, 4> color) const noexcept;
		void begin_debug_utils_label(std::string_view labelName, std::array<float, 4> color) const noexcept;
		void end_debug_utils_label() const noexcept;

		[[nodiscard]] Type get_type() const noexcept;
		[[nodiscard]] uint32_t get_queue_family_index() const noexcept;
		[[nodiscard]] float get_priority() const noexcept;
		[[nodiscard]] bool is_present_capable() const noexcept;
		
	private:

		Type m_type;
		uint32_t m_queueFamilyIndex {~0u};
		float m_priority {0.f};
		bool m_presentCapable {false};
	};
	template<typename T>
	using QueueContainer = magic_enum::containers::array<Queue::Type,  std::vector<T>>;
}
