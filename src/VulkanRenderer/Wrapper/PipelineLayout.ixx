module;

#include <expected>
#include <span>

#include "volk.h"

export module NYANVulkanWrapper:PipelineLayout;
import :Object;
import :Error;
import :LogicalDeviceWrapper;
import :DeletionQueue;
import :DescriptorSetLayout;

export namespace nyan::vulkan::wrapper
{
	class PipelineLayout : public Object<VkPipelineLayout>
	{
	public:

		PipelineLayout(PipelineLayout&) = delete;
		PipelineLayout(PipelineLayout&&) noexcept;

		PipelineLayout& operator=(PipelineLayout&) = delete;
		PipelineLayout& operator=(PipelineLayout&&) noexcept;

		~PipelineLayout() noexcept;

		[[nodiscard]] static std::expected<PipelineLayout, Error> create(const LogicalDeviceWrapper& deviceWrapper, DeletionQueue& deletionQueue, std::span<DescriptorSetLayout> descriptorSetLayouts) noexcept;

		static constexpr uint32_t pushConstantSize = 128;
	private:
		PipelineLayout(const LogicalDeviceWrapper& deviceWrapper, VkPipelineLayout pipelineLayout, DeletionQueue& deletionQueue) noexcept;

		DeletionQueue& r_deletionQueue;
	};
}
