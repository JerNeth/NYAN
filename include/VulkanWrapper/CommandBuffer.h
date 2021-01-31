#pragma once
#ifndef COMMANDBUFFER_H
#define COMMANDBUFFER_H
#include "VulkanIncludes.h"
namespace Vulkan {
	class LogicalDevice;
	class CommandBuffer {
	public:
		CommandBuffer(LogicalDevice& parent);
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);
		void draw_indexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
		void bind_uniform_buffer(uint32_t set, uint32_t binding, uint32_t offset); //TODO: Add Buffer to bind
		void bind_texture(uint32_t set, uint32_t binding); // TODO: Add Sampler and Image
		void begin_render_pass();
		void end_render_pass();
		void submit_secondary_command_buffer(const CommandBuffer& secondary);
		void next_subpass(VkSubpassContents subpass);
	private:
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_device;
		VkCommandBuffer m_commandBuffer;
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
	};
}
#endif