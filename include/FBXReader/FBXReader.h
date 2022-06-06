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
namespace Utility {

	class FBXReader {
	public:
		FBXReader();
		~FBXReader();
		void parse_meshes(std::string fbxFile, std::vector<nyan::Mesh>& retMeshes, std::vector<nyan::MaterialData>& retMats);
	private:
		void parse_mesh(fbxsdk::FbxNode* node, std::vector<nyan::Mesh>& retMeshes);
		void parse_meshes(fbxsdk::FbxNode* parent, std::vector<nyan::Mesh>& retMeshes);

		FbxManager* sdkManager;
		FbxIOSettings* ios;
	};
}
#endif //!FBXREADER_H