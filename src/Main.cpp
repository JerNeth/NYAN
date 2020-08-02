
#include <iostream>
#include <imgui.h>
#include "VkWrapper.h"
#include <glfwWrapper.h>


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
		instance.setup_device();
		instance.create_swapchain();
		main_loop();

		while (!window.should_close())
		{
			//window.swap_buffers();
			glfwPollEvents();
		}
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	return 0;
}