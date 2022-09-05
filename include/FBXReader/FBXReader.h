#pragma once
#ifndef FBXREADER_H
#define FBXREADER_H
#undef PropertyNotify

#ifdef __INTELLISENSE__
#pragma warning disable 26495
#pragma warning disable 26451
#endif
#include <fbxsdk.h>
#include "Renderer/Mesh.h"
#include "Renderer/Material.h"
#include "Renderer/Light.h"
#include <filesystem>
namespace Utility {

	class FBXReader {
	public:
		FBXReader(const std::filesystem::path& directory = std::filesystem::current_path());
		~FBXReader();
		void parse_meshes(const std::filesystem::path& fbxFile, std::vector<nyan::Mesh>& retMeshes, std::vector<nyan::MaterialData>& retMats, std::vector<nyan::LightParameters>& retLights);
	private:
		void parse_node(fbxsdk::FbxNode* node, std::vector<nyan::Mesh>& retMeshes, std::vector<nyan::LightParameters>& retLights);
		void parse_meshes(fbxsdk::FbxNode* parent, std::vector<nyan::Mesh>& retMeshes, std::vector<nyan::LightParameters>& retLights);

		std::filesystem::path m_directory;
		FbxManager* sdkManager;
		FbxIOSettings* ios;
	};
}
#endif //!FBXREADER_H