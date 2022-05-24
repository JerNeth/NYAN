#pragma once
#ifndef RDTEXTUREMANAGER_H
#define RDTEXTUREMANAGER_H
#include <filesystem>
#include "VkWrapper.h"
#include "LinAlg.h"
#include "LogicalDevice.h"
#include "Utility/DDSReader.h"
#include <Util>
namespace nyan {
	class TextureManager {
	private:
		struct Texture {
			vulkan::ImageHandle handle;
			Utility::TextureInfo info;
		};
	public:
		TextureManager(vulkan::LogicalDevice& device, bool streaming);
		vulkan::Image* request_texture(const std::string& name);
		uint32_t get_texture_idx(const std::string& name, const std::string& defaultTex = "white.png");
		void change_mip(const std::string& name, uint32_t targetMip);
		void set_minimum_mip_level(uint32_t mipLevel) {
			m_minimumMipLevel = mipLevel;
		}
	private:
		vulkan::ImageHandle create_image(const std::filesystem::path& file, uint32_t mipLevel = 0);
		vulkan::ImageHandle create_dds_image(const std::filesystem::path& file, uint32_t mipLevel = 0);

		vulkan::LogicalDevice& r_device;
		std::unordered_map<uint32_t, Texture> m_usedTextures;
		std::unordered_map<std::string, uint32_t> m_textureIndex;
		bool m_streaming;
		bool m_useSparse = false;
		uint32_t m_minimumMipLevel = 0;
		//std::vector<vulkan::ImageHandle> m_usedTextures;
	};
}
#endif !RDTEXTUREMANAGER_H-