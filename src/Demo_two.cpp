#include <iostream>
#include <Util>
#include "Application.h"
using namespace nyan;
int main() {
	auto name = "Demo";
	nyan::Application application(name);

	auto& device = application.get_device();
	auto& window = application.get_window();

	vulkan::ShaderManager shaderManager(device);


	nyan::Rendergraph rendergraph{ device };

	auto& pass = rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Graphics);
	pass.add_swapchain_attachment();
	rendergraph.build();

	nyan::ImguiRenderer imgui(device, shaderManager, pass, &window);
	application.each_frame_begin([&]()
	{
		imgui.next_frame();
	});
	application.each_update([&](std::chrono::nanoseconds dt)
	{
		//imgui.update();
	});
	application.each_frame_end([&rendergraph]() 
	{
		rendergraph.execute();
	});
	application.main_loop();


	return 0;
}