module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:CommandBuffer;
import :LogicalDeviceWrapper;
import :PipelineLayout;
import :Queue;
import :Object;
import :Error;

export namespace nyan::vulkan::wrapper
{
	class CommandPool;
	class ComputePipeline;
	class DescriptorSet;

	class PipelineBind 
	{
	public:

		void push_descriptor_set(uint32_t firstSet) const noexcept;
		void bind_descriptor_set(uint32_t firstSet, const DescriptorSet& set) const noexcept;
		template<typename T>
		void push_constants(const T& t) const noexcept
		{
			static_assert(sizeof(T) < PipelineLayout::pushConstantSize);
			r_device.vkCmdPushConstants(m_cmd, m_layout, VK_SHADER_STAGE_ALL, 0, sizeof(T), &t);
		}
	protected:
		PipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout, VkPipelineBindPoint bindPoint) noexcept;

		const LogicalDeviceWrapper& r_device;

		VkCommandBuffer m_cmd;
		VkPipelineLayout m_layout;
		VkPipelineBindPoint m_bindPoint;
	};

	class ComputePipelineBind : public PipelineBind
	{
	public:
		ComputePipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;

		void dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1) const noexcept;

	};

	class CommandBuffer : public Object<VkCommandBuffer>
	{
	public:
		enum class State : uint32_t
		{
			Initial,
			Recording,
			Executable,
			Pending,
			Invalid,
			Size
		};
		friend class CommandBufferAccessor;
	public:
		CommandBuffer(CommandBuffer&) = delete;
		CommandBuffer(CommandBuffer&&) noexcept;

		CommandBuffer& operator=(CommandBuffer&) = delete;
		CommandBuffer& operator=(CommandBuffer&&) noexcept;

		~CommandBuffer() noexcept;

		[[nodiscard]] std::expected<void, Error> begin(bool oneTimeSubmit = true) noexcept;
		[[nodiscard]] std::expected<void, Error> end() noexcept;

		//void begin_rendering(const VkRenderingInfo& info) noexcept;
		//void end_rendering() noexcept;


		//[[nodiscard]] GraphicsPipelineBind bind_graphics_pipeline(PipelineId pipelineIdentifier) noexcept;
		[[nodiscard]] ComputePipelineBind bind_pipeline(const ComputePipeline& pipeline) noexcept;
		//[[nodiscard]] RaytracingPipelineBind bind_raytracing_pipeline(PipelineId pipelineIdentifier) noexcept;

		void begin_region(const char* name, const float* color = nullptr) noexcept;
		void end_region() noexcept;
		[[nodiscard]] Queue::Type get_type() const noexcept;

	private:
		CommandBuffer(const LogicalDeviceWrapper& device, VkCommandBuffer handle, Queue::Type type) noexcept;

		State m_state;
		Queue::Type m_type;
	};

	class CommandBufferAccessor
	{
	public:
		friend class CommandPool;
	private:
		static void set_state(CommandBuffer& cmd, const CommandBuffer::State state) noexcept
		{
			cmd.m_state = state;
		}
		static CommandBuffer::State get_state(const CommandBuffer& cmd) noexcept
		{
			return cmd.m_state;
		}
	};
}
