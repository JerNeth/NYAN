#include <iostream>
#include <Util>
#include "Application.h"
using namespace nyan;
int main() {
	auto name = "Demo";
	nyan::Application application(name);
	//try {

	auto& device = application.get_device();
	auto& window = application.get_window();
	//window.change_mode(glfww::WindowMode::FullscreenWindowed);

	vulkan::ShaderManager shaderManager(device);


	nyan::Rendergraph rendergraph{ device };

	auto& pass = rendergraph.add_pass("Fullscreen - Pass", nyan::Renderpass::Type::Graphics);
	pass.add_swapchain_attachment();
	rendergraph.build();

	vulkan::GraphicsPipelineConfig config{
	.dynamicState = vulkan::defaultDynamicGraphicsPipelineState,
	.state = vulkan::defaultGraphicsPipelineState,
	.vertexInputCount = 0,
	.shaderCount = 2,
	.shaderInstances {
		shaderManager.get_shader_instance_id("fullscreen_vert"),
		shaderManager.get_shader_instance_id("fullscreen_frag"),
	},
	.pipelineLayout = device.get_bindless_pipeline_layout(),
	};
	config.dynamicState.cull_mode = VK_CULL_MODE_NONE;
	auto pipeline = pass.add_pipeline(config);

	pass.add_renderfunction([&](vulkan::CommandBufferHandle& cmd, nyan::Renderpass& pass)
		{

			VkViewport viewport{
			.x = 0,
			.y = 0,
			.width = static_cast<float>(device.get_swapchain_width()),
			.height = static_cast<float>(device.get_swapchain_height()),
			.minDepth = 0.0,
			.maxDepth = 1.0,
			};
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

			auto bind = cmd->bind_graphics_pipeline(pipeline);	

			bind.set_viewport(viewport);

			bind.set_scissor(scissor);
			bind.draw(3, 1, 0 , 0);
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