#pragma once
#ifndef RDMESHMANAGER_H
#define RDMESHMANAGER_H
#include "VkWrapper.h"
#include "LinAlg.h"
namespace nyan {
	class StaticMesh;
	class MeshManager {
	public:
		MeshManager(vulkan::LogicalDevice& device);
		StaticMesh* request_static_mesh();
	private:
		vulkan::LogicalDevice& r_device;
	};
}

#endif !RDMESHMANAGER_H