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
		TextureManager(vulkan::LogicalDevice& device, bool streaming);
		vulkan::Image* request_texture(const std::string& name);
		void change_mip(const std::string& name, uint32_t targetMip);
	private:
		vulkan::LogicalDevice& r_device;
		std::unordered_map<std::string, vulkan::ImageHandle> m_usedTextures;
		bool m_streaming;
		//std::vector<vulkan::ImageHandle> m_usedTextures;
	};
}
#endif !RDTEXTUREMANAGER_H-