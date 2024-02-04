module;

#include <expected>

export module NYANVulkanWrapper:CommandBuffer;
import :LogicalDeviceWrapper;
import :Queue;
import :Object;
import :Error;

export namespace nyan::vulkan::wrapper
{
	class CommandPool;
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
		//[[nodiscard]] ComputePipelineBind bind_compute_pipeline(PipelineId pipelineIdentifier) noexcept;
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
