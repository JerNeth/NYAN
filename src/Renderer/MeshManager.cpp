#include "Renderer/MeshManager.h"

using namespace vulkan;
using namespace nyan;

nyan::MeshManager::MeshManager(vulkan::LogicalDevice& device) :
	r_device(device)
{
}