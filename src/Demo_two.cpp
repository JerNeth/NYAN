﻿
#include <iostream>

//#include <imgui.h>
#include "VkWrapper.h"
#include <glfwWrapper.h>
#include <chrono>

#include <stb_image.h>
#include "LinAlg.h"
#include "Transform.h"
#include <new>
#include <thread>
using namespace std;
using namespace Math;


int main()
{
	/*
	unsigned int n = std::thread::hardware_concurrency();
	std::cout << n << " concurrent threads are supported.\n";
	std::cout << std::hardware_constructive_interference_size << " hardware_constructive_interference_size.\n";
	std::cout << std::hardware_destructive_interference_size << " hardware_destructive_interference_size .\n";
	*/
	int width = 1920, height = 1080;
	std::string applicationName{ "Demo" };
	std::string engineName{ "NYAN" };
	try {
		glfww::Library library;
		glfww::Window window(width, height, nullptr, nullptr, applicationName.c_str());
		//auto window = glfww::Window::create_full_screen(width, height);
		//auto window = glfww::Window::create_full_screen(width, height);
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		Vulkan::Instance instance(glfwExtensions, glfwExtensionCount, applicationName, engineName);		
		instance.setup_win32_surface(window.get_win32_window(), GetModuleHandle(nullptr));

		Vulkan::LogicalDevice device = instance.setup_device();
		device.demo_setup();
		int texWidth, texHeight, texChannels;
		stbi_uc* pixels = stbi_load("texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		device.create_texture_image(texWidth, texHeight, 4, reinterpret_cast<char*>(pixels));
		stbi_image_free(pixels);
		
		device.create_stuff();
		device.create_sync_objects();
		int frame = 0;
		auto start = chrono::steady_clock::now();
		auto total = start - start;
		while (!window.should_close())
		{
			start = chrono::steady_clock::now();
			//window.swap_buffers();
			glfwPollEvents();
			if (!window.is_iconified()) {
				device.draw_frame();
				frame++;
				total += chrono::steady_clock::now() - start;
			}
		}
		auto end = chrono::steady_clock::now();
		std::cout << "Average: "<< chrono::duration_cast<chrono::microseconds>(total).count()/frame << "microseconds\n";
		device.wait_idle();
		device.demo_teardown();
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	return 0;
}