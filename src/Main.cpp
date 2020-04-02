#include "LinAlg.h"
#include <iostream>
#include "Transform.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glfwWrapper.h>
using namespace std;
using namespace bla;

void main_loop() {

}

int main()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	
	std::cout << extensionCount << endl;
	try {
		glfww::Library library;
		glfww::Window window = glfww::Window(1920, 1080);

		VkInstance_T* instance;
		VkApplicationInfo applicationInfo = {};
		applicationInfo.apiVersion;
		main_loop();
		while (!window.should_close())
		{
			//window.swap_buffers();
			glfwPollEvents();
		}
	}
	catch (const std::runtime_error& error) {
		std::cerr << error.what() << std::endl;
		//Probable cause: GLFW could not be loaded
		return EXIT_FAILURE;
	}
	
	return 0;
}