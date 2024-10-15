module;

//#include <expected>
//#include <string_view>

#include "magic_enum.hpp"

export module NYANRenderer:Resources;
import std;
import NYANVulkan;
import NYANData;
import :Error;

export namespace nyan::renderer
{
	class Resources
	{
	private:
		struct Data {
			vulkan::Instance instance;
			vulkan::LogicalDevice device;

		};
	public:
		enum class Feature : uint32_t {
			Validation,
			WSI,
		};
		using Features = nyan::bitset<magic_enum::enum_count<Feature>(), Feature>;

		[[nodiscard("must handle potential error")]] static std::expected<Resources, ResourceError> create(Features features, std::string_view applicationName) noexcept;
	private:
		Resources(Data data) noexcept;

		Data m_data;
	};
}