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

		std::expected<Renderer, RendererCreationError> create() noexcept;
	private:
		Renderer() noexcept;
	};
}
