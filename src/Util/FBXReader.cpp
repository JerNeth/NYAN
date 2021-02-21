#include "Utility/FBXReader.h"
#include "Renderer/Mesh.h"

Utility::FBXReader::FBXReader()
{
	sdkManager = FbxManager::Create();

	ios = FbxIOSettings::Create(sdkManager, IOSROOT);
	sdkManager->SetIOSettings(ios);
}

Utility::FBXReader::~FBXReader()
{
	ios->Destroy();
	sdkManager->Destroy();
}

std::vector<nyan::MeshData> Utility::FBXReader::parse_meshes(std::string fbxFile)
{
	FbxImporter* importer = FbxImporter::Create(sdkManager, "");

	if (!importer->Initialize(fbxFile.c_str(), -1, sdkManager->GetIOSettings()))
		return {};

	::FbxScene* scene = ::FbxScene::Create(sdkManager, "ImportScene");

	importer->Import(scene);

	importer->Destroy();
	nyan::MeshData ret;
	::FbxNode* rootNode = scene->GetRootNode();

	//std::cout << rootNode->GetChildCount() << '\n';

	auto node = rootNode->GetChild(0);/*
	std::cout << rootNode->GetName() << '\n';
	std::cout << node->GetName() << '\n';*/
	auto mesh = node->GetMesh();
	ret.name = node->GetName();

	//for (int i = 0; i < node->GetMaterialCount(); i++) {
	//	fbxsdk::FbxSurfaceMaterial* mat= node->GetMaterial(i);
	//	std::cout<< mat->GetName() << ' ';
	//	auto s = mat->ShadingModel.Get();
	//	assert(s == "Phong");
	//	auto phong = reinterpret_cast<fbxsdk::FbxSurfacePhong*>(mat);
	//	std::cout << phong->ShadingModel.Get() << ' ';
	//	//std::cout << phong->Diffuse. << '\n';
	//}

	FbxVector4* lControlPoints = mesh->GetControlPoints();/*
	std::cout << "Poly Count: " << mesh->GetPolygonCount() << "\n";*/
	auto* findices = mesh->GetPolygonVertices();/*
	std::cout << "Polygon Vertices Count: " << mesh->GetPolygonVertexCount() << "\n";*/
	
	
	ret.vertices = std::vector<nyan::StaticMesh::Vertex>();
	if(mesh->GetControlPointsCount() >= UINT16_MAX)
		ret.indices = std::vector<uint32_t>();
	else
		ret.indices = std::vector<uint16_t>();
	auto& vertices = ret.get_static_vertices();
	//assert(mesh->GetControlPointsCount() < UINT16_MAX);
	vertices.reserve(mesh->GetControlPointsCount());
	for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
		nyan::StaticMesh::Vertex vertex;
		for (int j = 0; j < 3; j++)
			vertex.pos[j] = lControlPoints[i].mData[j];
		vertices.push_back(vertex);
	}
	fbxsdk::FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvs;
	mesh->GetTextureUV(&uvs);

	if (mesh->GetControlPointsCount() >= UINT16_MAX) {
		auto& indices = ret.get_indices32();
		indices.reserve(mesh->GetPolygonVertexCount());
		for (int i = 0; i < mesh->GetPolygonCount(); i++) {
			auto polyIndex = mesh->GetPolygonVertexIndex(i);
			int k = mesh->GetPolygonSize(i);
			for (int j = 0; j < k; j++) {
				auto uvIdx = mesh->GetTextureUVIndex(i, j);
				auto uv = uvs->operator[](uvIdx);
				auto& vertex = vertices[findices[polyIndex + j]];
				vertex.uv.x() = uv[0] * UINT16_MAX;
				vertex.uv.y() = (1.0f - uv[1]) * UINT16_MAX;
			}
			assert(k == 4 || k == 3);
			//TODO Triangulate non-square polygons
			indices.push_back(findices[polyIndex + 0]);
			indices.push_back(findices[polyIndex + 1]);
			indices.push_back(findices[polyIndex + 2]);
			if (k == 4) {
				indices.push_back(findices[polyIndex + 2]);
				indices.push_back(findices[polyIndex + 3]);
				indices.push_back(findices[polyIndex + 0]);
			}
		}
	}
	else {
		auto& indices = ret.get_indices16();
		indices.reserve(mesh->GetPolygonVertexCount());
		for (int i = 0; i < mesh->GetPolygonCount(); i++) {
			auto polyIndex = mesh->GetPolygonVertexIndex(i);
			int k = mesh->GetPolygonSize(i);
			for (int j = 0; j < k; j++) {
				auto uvIdx = mesh->GetTextureUVIndex(i, j);
				auto uv = uvs->operator[](uvIdx);
				auto& vertex = vertices[findices[polyIndex + j]];
				vertex.uv.x() = uv[0] * UINT16_MAX;
				vertex.uv.y() = (1.0f - uv[1]) * UINT16_MAX;
			}
			assert(k == 4 || k == 3);
			//TODO Triangulate non-square polygons
			indices.push_back(findices[polyIndex + 0]);
			indices.push_back(findices[polyIndex + 1]);
			indices.push_back(findices[polyIndex + 2]);
			if (k == 4) {
				indices.push_back(findices[polyIndex + 2]);
				indices.push_back(findices[polyIndex + 3]);
				indices.push_back(findices[polyIndex + 0]);
			}
		}
	}
	/*for (int i = 0; i < mesh->GetPolygonVertexCount(); i++) {
		std::cout << indices[i] << " ";
	}
	std::cout << "\n";*/

	/*for (int i = 0; i < mesh->GetPolygonCount(); i++) {
		std::cout << "(" << mesh->GetPolygonSize(i) << "):";
		auto polyIndex = mesh->GetPolygonVertexIndex(i);
		for (int j = 0; j < mesh->GetPolygonSize(i); j++) {
			std::cout << indices[polyIndex+j] << ' ';
		}
		std::cout <<'\n';
	}*/


	/*for (int i = 0; i < mesh->GetControlPointsCount(); i++) {
		for (int j = 0; j < 4; j++)
			std::cout << lControlPoints[i].mData[j] << ' ';
		std::cout  << '\n';
	}*/
	return { ret };
}
