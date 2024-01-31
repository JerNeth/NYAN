module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:DescriptorSetLayout;
import :Error;
import :Object;
import :DeletionQueue;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class DescriptorSetLayout : public Object<VkDescriptorSetLayout>
	{
	public:
		struct DescriptorInfo {
			uint32_t storageBufferCount{ 0 };
			uint32_t uniformBufferCount{ 0 };
			uint32_t samplerCount{ 0 };
			uint32_t sampledImageCount{ 0 };
			uint32_t storageImageCount{ 0 };
			uint32_t accelerationStructureCount{ 0 };
		};
	public:
		static constexpr inline uint32_t storageBufferBinding = 0;
		static constexpr inline uint32_t uniformBufferBinding = 1;
		static constexpr inline uint32_t samplerBinding = 2;
		static constexpr inline uint32_t sampledImageBinding = 3;
		static constexpr inline uint32_t storageImageBinding = 4;
		static constexpr inline uint32_t accelerationStructureBinding = 5;

	public:
		DescriptorSetLayout(DescriptorSetLayout&) = delete;
		DescriptorSetLayout(DescriptorSetLayout&&) noexcept;

		DescriptorSetLayout& operator=(DescriptorSetLayout&) = delete;
		DescriptorSetLayout& operator=(DescriptorSetLayout&&) noexcept;

		~DescriptorSetLayout() noexcept;

		const DescriptorInfo& get_info() const noexcept;

		static VkDescriptorType bindless_binding_to_type(const uint32_t binding) noexcept;

		static [[nodiscard]] std::expected<DescriptorSetLayout, Error> create(LogicalDevice& device, DescriptorInfo info) noexcept;

	private:
		DescriptorSetLayout(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorSetLayout layout, DeletionQueue& deletionQueue, DescriptorInfo info) noexcept;

		DeletionQueue& r_deletionQueue;
		DescriptorInfo m_info;
	};
}
