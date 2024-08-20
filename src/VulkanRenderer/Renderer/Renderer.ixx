module;

#include <expected>

export module NYANVulkanRenderer:Renderer;
import NYANVulkanWrapper;
import :Error;

export namespace nyan::vulkan::renderer
{
	class Renderer
	{
	public:

		[[nodiscard("must handle potential error")]] static std::expected<Renderer, RendererCreationError> create() noexcept;
	private:
		Renderer() noexcept;
	};
}
