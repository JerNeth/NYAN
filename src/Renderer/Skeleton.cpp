#include "Renderer/Skeleton.h"
using namespace nyan;
using namespace vulkan;

void nyan::Skeleton::bind([[maybe_unused]] vulkan::CommandBufferHandle& cmd)
{
	assert(buffer);
	assert((offset % 256) == 0);
}