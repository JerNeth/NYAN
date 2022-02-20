#pragma once
#ifndef FBXREADER_H
#define FBXREADER_H
#undef PropertyNotify

#ifdef __INTELLISENSE__
#pragma warning disable 26495
#pragma warning disable 26451
#endif
#include <fbxsdk.h>
#include <Renderer/MeshLoader.h>
namespace Utility {
	//TODO move this into an importer class
	//So that modules don't depend on this class
	//      this		other modules
	//		  \			/
	//		  _\|	  |/_
	//		Importer class

	class FBXReader {
	public:
		FBXReader();
		~FBXReader();
		std::vector<nyan::MeshData> parse_meshes(std::string fbxFile, bool tangentSpace);
	private:
		FbxManager* sdkManager;
		FbxIOSettings* ios;
	};
}
#endif //!FBXREADER_H