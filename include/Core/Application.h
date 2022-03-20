#pragma once
#ifndef NYANAPPLICATION_H
#define NYANAPPLICATION_H
#include "VkWrapper.h"
#include "VulkanRenderer"
#include "glfwWrapper.h"
#include "Settings.h"
namespace nyan {
	struct InitializationError : public std::exception {

		InitializationError([[maybe_unused]] const char* msg) : std::exception(){

		}
	};
	class Application {
	public:
		Application(const std::string& name);
		//Remove these functions and refactor
		vulkan::LogicalDevice& get_device();
		glfww::Window& get_window();
		int get_width();
		int get_height();

		void next_frame();
		void end_frame();
		void main_loop();
		void each_frame_begin(std::function<void()> beginFrame);
		void each_frame_end(std::function<void()> endFrame);
		void each_update(std::function<void(std::chrono::nanoseconds)> update);
		void update();
	private:
		bool setup_glfw();
		bool setup_glfw_window();
		bool setup_vulkan_instance();
		bool setup_vulkan_device();
		bool setup_vulkan_surface();

		const std::string m_name;
		const std::string m_engineName{ "NYAN" };
		Settings m_settings;

		std::unique_ptr<glfww::Library> m_glfwLibrary;
		std::unique_ptr<glfww::Monitor> m_monitor;
		std::unique_ptr<glfww::Window> m_window;
		std::unique_ptr<vulkan::Instance> m_vulkanInstance;
		std::unique_ptr<vulkan::LogicalDevice> m_vulkanDevice;
		std::unique_ptr<vulkan::WindowSystemInterface> m_windowSystemInterface;

		std::vector< std::function<void(std::chrono::nanoseconds)>> m_updateFunctions;
		std::vector< std::function<void()>> m_beginFrameFunctions;
		std::vector< std::function<void()>> m_endFrameFunctions;
		std::chrono::time_point<std::chrono::steady_clock> lastUpdate;
	};
}

#endif