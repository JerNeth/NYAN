#pragma once
#ifndef NYANSETTINGS_H
#define NYANSETTINGS_H
#include <unordered_map>
#include <string>
#include <variant>
namespace nyan {
	
	enum class Setting {
		Width,
		Height,
		WindowMode,
		Size
	};
	constexpr std::array<const char*, static_cast<size_t>(Setting::Size)> SettingNames{
		"Width",
		"Height",
		"WindowMode"
	};
	class Settings {
		
	public:
		Settings(const std::string &filename = "defaultSettings.ini");
		~Settings();
		template<typename T>
		const T& get_or_default(Setting key,const T& defaultValue) const
		{
			
			if (const auto& t = m_settings.find(key); t != m_settings.end()) {
				assert(std::holds_alternative<T>(t->second));
				return std::get<T>(t->second);
			}
			else {
				return defaultValue;
			}
		}
		template<typename T>
		void set_value(Setting key, const T& t) {
			m_settings[key] = t;
		}
	private:
		std::unordered_map<Setting, std::variant<int, std::string, float>> m_settings;
	};
}
#endif