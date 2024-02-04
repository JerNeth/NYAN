module;

#include <expected>

#include "volk.h"

export module NYANVulkanWrapper:Pipeline;
import :LogicalDeviceWrapper;
import :PipelineCache;
import :PipelineLayout;
import :Object;
import :Shader;
import :Error;


export namespace nyan::vulkan::wrapper
{

	class GraphicsPipeline : public Object<VkPipeline>
	{
	public:
		GraphicsPipeline(GraphicsPipeline&) = delete;
		GraphicsPipeline(GraphicsPipeline&&) noexcept;

		GraphicsPipeline& operator=(GraphicsPipeline&) = delete;
		GraphicsPipeline& operator=(GraphicsPipeline&&) noexcept;

		~GraphicsPipeline() noexcept;

		[[nodiscard]] static std::expected<GraphicsPipeline, Error> create(const LogicalDeviceWrapper& device, PipelineCache& pipelineCache) noexcept;
	private:
		GraphicsPipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept;

	};

	class ComputePipeline : public Object<VkPipeline>
	{
	public:
		ComputePipeline(ComputePipeline&) = delete;
		ComputePipeline(ComputePipeline&&) noexcept;

		ComputePipeline& operator=(ComputePipeline&) = delete;
		ComputePipeline& operator=(ComputePipeline&&) noexcept;

		~ComputePipeline() noexcept;

		[[nodiscard]] static std::expected<ComputePipeline, Error> create(const LogicalDeviceWrapper& device, const PipelineLayout& layout, const ShaderInstance& computeShader, PipelineCache& pipelineCache) noexcept;
	private:
		ComputePipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept;

	};

	class RayTracingPipeline : public Object<VkPipeline>
	{
	public:
		RayTracingPipeline(RayTracingPipeline&) = delete;
		RayTracingPipeline(RayTracingPipeline&&) noexcept;

		RayTracingPipeline& operator=(RayTracingPipeline&) = delete;
		RayTracingPipeline& operator=(RayTracingPipeline&&) noexcept;

		~RayTracingPipeline() noexcept;

		[[nodiscard]] static std::expected<RayTracingPipeline, Error> create(const LogicalDeviceWrapper& device, PipelineCache& pipelineCache) noexcept;

	private:
		RayTracingPipeline(const LogicalDeviceWrapper& device, VkPipeline handle) noexcept;

	};

}
