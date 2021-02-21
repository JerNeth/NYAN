#pragma once
#ifndef FBXREADER_H
#define FBXREADER_H
#include <fbxsdk.h>
#include <Renderer\MeshLoader.h>
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
		std::vector<nyan::MeshData> parse_meshes(std::string fbxFile);
	private:
		FbxManager* sdkManager;
		FbxIOSettings* ios;
	};
}
#endif //!FBXREADER_H