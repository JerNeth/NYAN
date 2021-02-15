#pragma once
#ifndef RDTEXTUREMANAGER_H
#define RDTEXTUREMANAGER_H
#include "VkWrapper.h"
#include "LinAlg.h"
#include "LogicalDevice.h"
#include "Utility.h"
namespace nyan {
	class TextureManager {
	public:
		TextureManager(vulkan::LogicalDevice& device);
		vulkan::Image* request_texture(const std::string& name);
	private:
		vulkan::LogicalDevice& r_device;
		std::unordered_map<std::string, vulkan::ImageHandle> m_usedTextures;
		//std::vector<vulkan::ImageHandle> m_usedTextures;
	};
}
#endif !RDTEXTUREMANAGER_H