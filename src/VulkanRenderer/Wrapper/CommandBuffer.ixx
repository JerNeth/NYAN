module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:CommandBuffer;
import :Buffer;
import :Error;
import :LogicalDeviceWrapper;
import :Object;
import :Pipeline;
import :PipelineLayout;
import :Queue;

export namespace nyan::vulkan::wrapper
{
	class CommandPool;
	class DescriptorSet;

	class PipelineBind 
	{
	public:

		void push_descriptor_set(uint32_t firstSet, const StorageBuffer& buffer, VkDeviceSize offset = 0, VkDeviceSize range = VK_WHOLE_SIZE) const noexcept;
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
		friend class CommandBuffer;

		void dispatch(uint32_t groupCountX = 1, uint32_t groupCountY = 1, uint32_t groupCountZ = 1) const noexcept;

	private:
		ComputePipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;

	};

	class GraphicsPipelineBind : public PipelineBind
	{
	public:
		void set_viewport(uint16_t width, uint16_t height, float minDepth = 0.f, float maxDepth = 1.f) const noexcept;
		void set_scissor(uint16_t width, uint16_t height, uint16_t x = 0, uint16_t y = 0) const noexcept;
	protected:
		GraphicsPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;
	};

	class VertexPipelineBind : public GraphicsPipelineBind
	{
	public:
		friend class CommandBuffer;

	private:
		VertexPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;
	};

	class MeshPipelineBind : public GraphicsPipelineBind
	{
	public:
		friend class CommandBuffer;

	private:
		MeshPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;
	};

	class RayTracingPipelineBind : public PipelineBind
	{
	public:
		friend class CommandBuffer;

	private:
		RayTracingPipelineBind(const LogicalDeviceWrapper& device, VkCommandBuffer cmd, VkPipelineLayout layout) noexcept;
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


		[[nodiscard]] VertexPipelineBind bind_vertex_pipeline(const VertexShaderGraphicsPipeline& pipeline, GraphicsPipeline::DynamicStates dynamicDefaultsMask = ~GraphicsPipeline::DynamicStates{}) noexcept;
		[[nodiscard]] ComputePipelineBind bind_pipeline(const ComputePipeline& pipeline) noexcept;
		//[[nodiscard]] RaytracingPipelineBind bind_raytracing_pipeline(PipelineId pipelineIdentifier) noexcept;

		void begin_region(const char* name, const float* color = nullptr) noexcept;
		void end_region() noexcept;
		[[nodiscard]] Queue::Type get_type() const noexcept;

	private:
		CommandBuffer(const LogicalDeviceWrapper& device, VkCommandBuffer handle, Queue& queue) noexcept;

		Queue& r_queue;
		State m_state;
	};

	class CommandBufferAccessor
	{
	public:
		friend class CommandPool;
		friend class Queue;
	private:
		static CommandBuffer::State& state(CommandBuffer& cmd) noexcept
		{
			return cmd.m_state;
		}
		static const CommandBuffer::State& state(const CommandBuffer& cmd) noexcept
		{
			return cmd.m_state;
		}

		static CommandBuffer create(auto&&... args) noexcept 
		{
			return CommandBuffer{std::forward<decltype(args)>(args)...};
		}
	};
}
