module;

//#include <expected>

export module NYANRenderer:PipelineBuilder;
import std;
import NYANVulkan;
import :Error;

export namespace nyan::renderer
{
	class PipelineBuilder
	{
	public:
		[[nodiscard("must handle potential error")]] static std::expected<PipelineBuilder, Error> create() noexcept;
	private:
		PipelineBuilder() noexcept;

	};
}
