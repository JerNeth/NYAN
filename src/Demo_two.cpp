#include <iostream>
#include <fbxsdk.h>
#include <Util>
#include "Application.h"
#include "Utility/FBXReader.h"
using namespace nyan;
int main() {
	auto name = "Demo";
	nyan::Application application(name);
	//try {

	auto& device = application.get_device();
	auto& window = application.get_window();
	window.change_mode(glfww::WindowMode::FullscreenWindowed);

	vulkan::ShaderManager shaderManager(device);


	nyan::Rendergraph rendergraph{ device };

	auto& pass = rendergraph.add_pass("Imgui-Pass", nyan::Renderpass::Type::Graphics);
	pass.add_swapchain_attachment();
	rendergraph.build();
	vulkan::GraphicsPipelineConfig config{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::alphaBlendedGraphicsPipelineState,
	.vertexInputCount = 0,
	.shaderCount = 2,
	.shaderInstances {
		shaderManager.get_shader_instance_id("fullscreen_vert"),
		shaderManager.get_shader_instance_id("fullscreen_frag"),
	},
	.pipelineLayout = device.get_bindless_pipeline_layout(),
	};
	auto pipeline = pass.add_pipeline(config);

	pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd, nyan::Renderpass& pass) 
	{
		auto bind = cmd->bind_graphics_pipeline(pipeline);	
		VkViewport viewport{
		.x = 0,
		.y = 0,
		.width = static_cast<float>(device.get_swapchain_width()),
		.height = static_cast<float>(device.get_swapchain_height()),
		.minDepth = 0,
		.maxDepth = 1,
		};
		bind.set_viewport(viewport);
		VkRect2D scissor{
		.offset {
			.x = (int32_t)(0),
			.y = (int32_t)(0),
		},
		.extent {
			.width = (uint32_t)(viewport.width),
			.height = (uint32_t)(viewport.height),
		}
		};
		bind.set_scissor(scissor);
		bind.draw(3, 1, 0 , 0);
	});
	//nyan::ImguiRenderer imgui(device, shaderManager, pass, &window);

	application.each_frame_begin([&]()
	{
		//imgui.next_frame();
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
	//}
	//catch (const std::runtime_error& error) {
	//	std::cerr << error.what() << std::endl;
	//	return EXIT_FAILURE;
	//}

	return 0;
}