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
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance);
	private:
		/// *******************************************************************
		/// Member variables
		/// *******************************************************************
		LogicalDevice& r_parent;
		VkCommandBuffer m_commandBuffer;
		/// *******************************************************************
		/// Private functions
		/// *******************************************************************
	};
}
#endif