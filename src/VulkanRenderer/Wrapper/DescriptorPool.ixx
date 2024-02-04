module;

#include <expected>
#include <vector>

#include "volk.h"

export module NYANVulkanWrapper:DescriptorPool;
import :DescriptorSetLayout;
import :DescriptorSet;
import :Error;
import :Object;

export namespace nyan::vulkan::wrapper
{
	class LogicalDevice;
	class DescriptorPool : public Object<VkDescriptorPool>
	{
	public:
		DescriptorPool(DescriptorPool&) = delete;
		DescriptorPool(DescriptorPool&& other) noexcept;

		DescriptorPool& operator=(DescriptorPool&) = delete;
		DescriptorPool& operator=(DescriptorPool&& other) noexcept;

		~DescriptorPool() noexcept;

		std::vector< DescriptorSet>& get_sets() noexcept;

		/**
		 * \brief Creates a descriptor pool and automatically allocates numDescriptorSets amount of descriptor sets, not meant to be reset
		 * \param device 
		 * \param layout 
		 * \param numDescriptorSets 
		 * \return 
		 */
		static [[nodiscard]] std::expected<DescriptorPool, Error> create(LogicalDevice& device, const DescriptorSetLayout& layout, uint32_t numDescriptorSets) noexcept;
	private:
		DescriptorPool(const LogicalDeviceWrapper& deviceWrapper, VkDescriptorPool handle, DeletionQueue& deletionQueue, std::vector<DescriptorSet>& sets) noexcept;
		void reset() noexcept;

		DeletionQueue& r_deletionQueue;

		std::vector< DescriptorSet> m_sets;
	};
}
