module;

//#include <expected>
//#include <filesystem>

#include "magic_enum.hpp"

export module NYANRenderer:Settings;
import std;

export namespace nyan::renderer
{
	enum class Setting {
		VSync,
		Hdr,
		LowLatency,
		RenderScale
	};
	union SettingValue {
		uint64_t u;
		double f;
	};
	template<typename T>
	concept ScopedEnum = std::is_scoped_enum_v<T>;

	template<ScopedEnum T>
	class TSettings
	{
	public:
		class Error {
		public:
			enum class Type
			{
				UnknownError
			};
		public:
			Error() noexcept {};
			Error(Type type) noexcept {};
		};
	private:
		static constexpr size_t settingsCount = magic_enum::enum_count<T>();
		using settingsDataType = std::array<SettingValue, settingsCount>;
	public:
		constexpr TSettings() noexcept = default;

		[[nodiscard("must handle potential error")]] std::expected<void, TSettings::Error> store_to_disk(const std::filesystem::path& path) noexcept 
		{
			enum_for_each<T>([&](auto e) 
				{
					auto name = enum_name<T>();
				});
		}

		[[nodiscard("must handle potential error")]] static std::expected<TSettings, TSettings::Error> create(const std::filesystem::path& path) noexcept 
		{

		}
	private:
		TSettings(settingsDataType data) noexcept;


		settingsDataType m_data;
	};
	using Settings = TSettings<Setting>;
}
