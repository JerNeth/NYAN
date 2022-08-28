#ifndef UTEXCEPTIONS_H
#define UTEXCEPTIONS_H
#pragma once
#include <source_location>
#include <filesystem>
#include <format>
#include <exception>
namespace Utility {
	class FeatureNotSupportedException : public std::exception {
	public:
		FeatureNotSupportedException(const std::source_location location = std::source_location::current())
			: m_msg(std::format("{}: {} [{}]\n: Feature not supported by Device", location.file_name(), location.function_name(), location.line())) 
		{
		}
		const char* what() const override {
			return m_msg.c_str();
			
		}
	private:
		std::string m_msg{};
	};
	class VulkanException : public std::exception {
	public:
		VulkanException(VkResult errorCode, const std::source_location location = std::source_location::current())
		{
			switch (errorCode) {
			case VK_PIPELINE_COMPILE_REQUIRED_EXT:
				assert(false);
				//shouldnt be here
				m_msg = std::format("{}({}): {}: Vulkan error: Pipeline Compile Required", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_OUT_OF_DATE_KHR:
				assert(false);
				//shouldnt be here
				m_msg = std::format("{}({}): {}: Vulkan error: Swapchain out of date", location.file_name(), location.line(), location.function_name());
				break;
			case VK_NOT_READY:
				assert(false);
				//shouldnt be here
				m_msg = std::format("{}({}): {}: Vulkan error: Swapchain not ready", location.file_name(), location.line(), location.function_name());
				break;
			case VK_SUBOPTIMAL_KHR:
				assert(false);
				//shouldnt be here
				m_msg = std::format("{}({}): {}: Vulkan error: Swapchain suboptimal", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
				m_msg = std::format("{}({}): {}: Vulkan error: Native window already in use", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_SURFACE_LOST_KHR:
				m_msg = std::format("{}({}): {}: Vulkan error: Surface lost", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
				m_msg = std::format("{}({}): {}: Vulkan error: Full screen exclusive mode lost", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_INVALID_SHADER_NV:
				m_msg = std::format("{}({}): {}: Vulkan error: Invalid shader Nvidia", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_FRAGMENTATION_EXT:
				m_msg = std::format("{}({}): {}: Vulkan error: Fragmentation error", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_TOO_MANY_OBJECTS:
				m_msg = std::format("{}({}): {}: Vulkan error: Too many objects", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_INCOMPATIBLE_DRIVER:
				m_msg = std::format("{}({}): {}: Vulkan error: Incompatible driver", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_LAYER_NOT_PRESENT:
				m_msg = std::format("{}({}): {}: Vulkan error: Layer not present", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_INITIALIZATION_FAILED:
				m_msg = std::format("{}({}): {}: Vulkan error: Initialization failed", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_FEATURE_NOT_PRESENT:
				m_msg = std::format("{}({}): {}: Vulkan error: Feature not present", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_DEVICE_LOST:
				m_msg = std::format("{}({}): {}: Vulkan error: Device lost", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				m_msg = std::format("{}({}): {}: Vulkan error: Out of host memory", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				m_msg = std::format("{}({}): {}: Vulkan error: Out of device memory", location.file_name(), location.line(), location.function_name());
				break;
			case VK_ERROR_EXTENSION_NOT_PRESENT:
				m_msg = std::format("{}({}): {}: Vulkan error: Extension not present", location.file_name(), location.line(), location.function_name());
				break;
			default:
				m_msg = std::format("{}({}): {}: Vulkan error: {:#x}", location.file_name(), location.line(), location.function_name(), static_cast<int>(errorCode));
				break;
			}
		}
		const char* what() const override {
			return m_msg.c_str();
		}
	private:
		std::string m_msg{};
	};
	class DeviceLostException : public std::exception {
	public:
		DeviceLostException(const std::string& msg, const std::source_location location = std::source_location::current())
			: m_msg(std::format("{}({}): {}: Device Lost: {}", location.file_name(), location.line(), location.function_name(), msg))
		{
		}
		const char* what() const override {
			return m_msg.c_str();
		}
	private:
		std::string m_msg{};
	};
	class DevicePropertyException : public std::exception {
	public:
		DevicePropertyException(const std::string& msg, const std::source_location location = std::source_location::current())
			: m_msg(std::format("{}({}): {}: Requested property exceeds supported property: {}", location.file_name(), location.line(), location.function_name(), msg))
		{
		}
		const char* what() const override {
			return m_msg.c_str();
		}
	private:
		std::string m_msg{};
	};
	class FileNotFoundException : public std::exception {
	public:
		FileNotFoundException(const std::filesystem::path& file, const std::source_location location = std::source_location::current()) 
			: m_msg(std::format("{}({}): {}: File does not exist: \"{}\"", location.file_name(), location.line(), location.function_name(), file.string()))
		{
		}
		const char* what() const override {
			return m_msg.c_str();
		}
	private:
		std::string m_msg{};
	};
	class FileTypeNotSupportedException : public std::exception {
	public:
		FileTypeNotSupportedException(const std::filesystem::path& file, const std::source_location location = std::source_location::current())
			: m_msg(std::format("{}({}): {}: File type not supported: \"{}\"", location.file_name(), location.line(), location.function_name(), file.extension().string()))
		{

		}
		const char* what() const override {
			return m_msg.c_str();
		}
	private:
		std::string m_msg{};
	};
}

#endif !UTEXCEPTIONS_H