#pragma once
#ifndef NYANSETTINGS_H
#define NYANSETTINGS_H
#include <unordered_map>
#include <string>
#include <cassert>
#include <array>
#include <filesystem>
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
			DeviceIdx,
			Size
		};
		static constexpr std::array<const char*, static_cast<size_t>(Setting::Size)> SettingNames{
			"Width",
			"Height",
			"WindowMode",
			"Monitor",
			"DeviceIdx"
		};
		Settings(const std::filesystem::path& filename);
		~Settings();
		template<typename T>
		T get_or_default(const Setting& key, const T& defaultValue) const
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
		std::optional<T> get_value(const Setting& key) const {
			if constexpr (std::is_enum_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<intType>(t->second));
					return static_cast<T>(std::get<intType>(t->second));
				} 
			}
			else if constexpr (std::is_integral_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<intType>(t->second));
					return static_cast<T>(std::get<intType>(t->second));
				}
			}
			else if constexpr (std::is_floating_point_v<T>) {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<floatType>(t->second));
					return static_cast<T>(std::get<floatType>(t->second));
				}
			}
			else {
				if (const auto& t = m_settings.find(key); t != m_settings.end()) {
					assert(std::holds_alternative<T>(t->second));
					return std::get<T>(t->second);
				}
			}
			return std::nullopt;
		}
		bool has_value(const Setting& key) const
		{
			return m_settings.contains(key);
		}
		template<typename T>
		void set_value(const Setting& key, const T& t) {
			if constexpr (std::is_enum_v<T>) {
				m_settings[key] = static_cast<intType>(t);
			}
			else if constexpr (std::is_integral_v<T>) {
				m_settings[key] = static_cast<intType>(t);
			}
			else if constexpr (std::is_floating_point_v<T>) {
				m_settings[key] = static_cast<floatType>(t);
			}
			else {
				m_settings[key] = static_cast<T>(t);
			}
		}
	private:
		std::filesystem::path m_filename;
		std::unordered_map<Setting, std::variant<intType, std::string, floatType>> m_settings;
	};
}
#endif