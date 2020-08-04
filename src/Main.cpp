﻿
#include <iostream>
#include <imgui.h>
#include "VkWrapper.h"
#include <glfwWrapper.h>
#include <chrono>


#include "LinAlg.h"
#include "Transform.h"

using namespace std;
using namespace bla;


void main_loop() {

}

int main()
{
	int width = 1920, height = 1080;
	std::string applicationName{ "Demo" };
	std::string engineName{ "Simple Engine" };
	try {
		glfww::Library library;
		glfww::Window window(width, height, nullptr, nullptr, applicationName.c_str());
		//auto window = glfww::Window::create_full_screen(width, height);
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		vk::Instance instance(glfwExtensions, glfwExtensionCount, applicationName, engineName);		
		instance.setup_win32_surface(window.get_win32_window(), GetModuleHandle(nullptr));
		vk::LogicalDevice device = instance.setup_device();

		main_loop();
		int frame = 0;
		auto start = chrono::steady_clock::now();
		while (!window.should_close())
		{
			
			//window.swap_buffers();
			glfwPollEvents();
			device.draw_frame();
			frame++;
			
		}
		auto end = chrono::steady_clock::now();
		std::cout << "Average: "<< chrono::duration_cast<chrono::microseconds>(end - start).count()/frame << "microseconds\n";
		device.wait_idle();
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	return 0;
}