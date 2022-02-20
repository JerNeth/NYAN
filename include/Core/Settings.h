#pragma once
#ifndef NYANSETTINGS_H
#define NYANSETTINGS_H
#include <unordered_map>
#include <string>
#include <variant>
namespace nyan {
	
	class Settings {
		using intType = int64_t;
		using floatType = double;
	public:
		enum class Setting {
			Width,
			Height,
			WindowMode,
			Monitor,
			Size
		};
		static constexpr std::array<const char*, static_cast<size_t>(Setting::Size)> SettingNames{
			"Width",
			"Height",
			"WindowMode",
			"Monitor"
		};
		Settings(const std::string &filename = "defaultSettings.ini");
		~Settings();
		template<typename T>
		T get_or_default(Setting key, const T& defaultValue) const
		{
			if constexpr (std::is_enum_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<intType>(t->second));
					return static_cast<T>(std::get<intType>(t->second));
				}
				else {
					return defaultValue;
				}
			}
			else if constexpr (std::is_integral_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<intType>(t->second));
					return static_cast<T>(std::get<intType>(t->second));
				}
				else {
					return defaultValue;
				}
			}
			else if constexpr (std::is_floating_point_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<floatType>(t->second));
					return static_cast<T>(std::get<floatType>(t->second));
				}
				else {
					return defaultValue;
				}
			}
			else {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<T>(t->second));
					return std::get<T>(t->second);
				}
				else {
					return defaultValue;
				}
			}
		}
		template<typename T>
		void set_value(Setting key, const T& t) {
			m_settings[key] = t;
		}
	private:
		std::string m_filename;
		std::unordered_map<Setting, std::variant<intType, std::string, double>> m_settings;
	};
}
#endif