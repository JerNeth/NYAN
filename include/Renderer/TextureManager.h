#pragma once
#ifndef RDTEXTUREMANAGER_H
#define RDTEXTUREMANAGER_H
#include "VkWrapper.h"
#include "LinAlg.h"
#include "LogicalDevice.h"
#include "Utility/DDSReader.h"
#include <Util>
namespace nyan {
	class TextureManager {
	public:
		TextureManager(vulkan::LogicalDevice& device, bool streaming);
		vulkan::Image* request_texture(const std::string& name);
		void change_mip(const std::string& name, uint32_t targetMip);
		void set_minimum_mip_level(uint32_t mipLevel) {
			m_minimumMipLevel = mipLevel;
		}
	private:
		vulkan::ImageHandle create_image(const std::string& name, uint32_t mipLevel = 0);

		vulkan::LogicalDevice& r_device;
		std::unordered_map<std::string, std::pair<vulkan::ImageHandle, Utility::TextureInfo>> m_usedTextures;
		bool m_streaming;
		bool m_useSparse = false;
		uint32_t m_minimumMipLevel = 0;
		//std::vector<vulkan::ImageHandle> m_usedTextures;
	};
}
#endif !RDTEXTUREMANAGER_H-