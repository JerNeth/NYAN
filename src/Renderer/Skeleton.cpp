#include "Renderer/Skeleton.h"
using namespace nyan;
using namespace vulkan;

void nyan::Skeleton::bind(vulkan::CommandBufferHandle& cmd)
{
	assert(buffer);
	assert((offset % 256) == 0);
	cmd->bind_uniform_buffer(0, 0, 0, *buffer, offset, 256);
}